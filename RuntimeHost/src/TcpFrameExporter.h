#pragma once

/**
 * TcpFrameExporter
 *
 * FrameExporter implementation that streams encoded JPEG frames over a local
 * TCP connection.  The exporter acts as a server: it listens on a configurable
 * port and accepts one client at a time.  When a client connects, every
 * WriteFrame() call encodes the raw BGRA8 pixels as JPEG, prepends a
 * FrameStreamHeader, and sends the bytes synchronously.
 *
 * STB_IMAGE_WRITE_IMPLEMENTATION must be defined in exactly one translation
 * unit before this header is included.
 */

#define WIN32_LEAN_AND_MEAN
#include "FrameExporter.h"
#include "FrameStreamProtocol.h"

#include <winsock2.h>
#include <ws2tcpip.h>

// stb_image_write is included here; the caller must define
// STB_IMAGE_WRITE_IMPLEMENTATION before including this header.
#include "stb_image/stb_image_write.h"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

class TcpFrameExporter : public FrameExporter
{
public:
	explicit TcpFrameExporter(uint16_t port = FRAME_STREAM_PORT)
	{
		WSADATA wsaData = {};
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		m_ListenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_ListenSocket == INVALID_SOCKET)
			return;

		int opt = 1;
		setsockopt(m_ListenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

		sockaddr_in addr{};
		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;

		if (::bind(m_ListenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
		    ::listen(m_ListenSocket, 1) != 0)
		{
			closesocket(m_ListenSocket);
			m_ListenSocket = INVALID_SOCKET;
			return;
		}

		m_AcceptThread = std::thread(&TcpFrameExporter::AcceptLoop, this);
		m_WorkerThread = std::thread(&TcpFrameExporter::WorkerLoop, this);
	}

	~TcpFrameExporter() override
	{
		m_Running = false;

		// Close listen socket to unblock the blocking accept() call in AcceptLoop
		if (m_ListenSocket != INVALID_SOCKET)
		{
			closesocket(m_ListenSocket);
			m_ListenSocket = INVALID_SOCKET;
		}

		CloseClient();

		// Wake the worker thread so it can observe m_Running == false and exit
		m_WorkCV.notify_all();
		if (m_WorkerThread.joinable())
			m_WorkerThread.join();

		if (m_AcceptThread.joinable())
			m_AcceptThread.join();

		WSACleanup();
	}

	void WriteFrame(const void* pData, uint64_t dataSize, uint32_t width, uint32_t height) override
	{
		// Early-out — avoid the copy if no client is connected
		if (m_ClientSocket.load(std::memory_order_acquire) == INVALID_SOCKET)
			return;

		// Copy pixel data into the pending slot and wake the worker thread.
		// If the worker is still encoding the previous frame, it is overwritten
		// here (latest-frame policy): the freshest frame is always preferred over
		// queuing stale ones for a real-time stream.
		{
			std::lock_guard<std::mutex> lock(m_WorkMutex);
			m_Pending.data.resize(dataSize);
			memcpy(m_Pending.data.data(), pData, dataSize);
			m_Pending.width    = width;
			m_Pending.height   = height;
			m_Pending.hasFrame = true;
		}
		m_WorkCV.notify_one();
	}

private:
	void AcceptLoop()
	{
		while (m_Running)
		{
			SOCKET client = ::accept(m_ListenSocket, nullptr, nullptr);
			if (client == INVALID_SOCKET)
				break; // listen socket was closed — time to exit

			// Disable Nagle's algorithm for lower latency on small sends
			int noDelay = 1;
			setsockopt(client, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));

			// Swap in the new client, closing any previous one
			SOCKET old = m_ClientSocket.exchange(client, std::memory_order_acq_rel);
			if (old != INVALID_SOCKET)
				closesocket(old);
		}
	}

	void WorkerLoop()
	{
		// Local frame slot — vectors persist across iterations to reuse allocations.
		// std::swap is used to steal ownership from m_Pending in O(1).
		struct LocalFrame
		{
			std::vector<uint8_t> data;
			uint32_t             width    = 0;
			uint32_t             height   = 0;
			bool                 hasFrame = false;
		} local;

		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_WorkMutex);
				m_WorkCV.wait(lock, [this] { return m_Pending.hasFrame || !m_Running; });

				if (!m_Running && !m_Pending.hasFrame)
					break;

				// Steal the pending frame — O(1) vector swap, no copy
				std::swap(local.data, m_Pending.data);
				local.width        = m_Pending.width;
				local.height       = m_Pending.height;
				local.hasFrame     = true;
				m_Pending.hasFrame = false;
			}

			// Lock released — encode and send without blocking WriteFrame
			EncodeAndSend(local.data.data(), local.data.size(), local.width, local.height);
		}
	}

	void EncodeAndSend(const uint8_t* pBGRA, size_t /*dataSize*/, uint32_t width, uint32_t height)
	{
		SOCKET sock = m_ClientSocket.load(std::memory_order_acquire);
		if (sock == INVALID_SOCKET)
			return;

		// Swizzle BGRA8 → RGB (3-channel; alpha discarded, R and B swapped)
		uint32_t nPixels = width * height;
		m_RgbScratch.resize(nPixels * 3u);
		for (uint32_t i = 0; i < nPixels; ++i)
		{
			m_RgbScratch[i * 3u + 0u] = pBGRA[i * 4u + 2u]; // R ← B
			m_RgbScratch[i * 3u + 1u] = pBGRA[i * 4u + 1u]; // G ← G
			m_RgbScratch[i * 3u + 2u] = pBGRA[i * 4u + 0u]; // B ← R
		}

		// JPEG encode into m_EncodedFrame
		m_EncodedFrame.clear();
		stbi_write_jpg_to_func(
		    [](void* ctx, void* data, int size) {
			    auto*          buf   = static_cast<std::vector<uint8_t>*>(ctx);
			    const uint8_t* bytes = static_cast<const uint8_t*>(data);
			    buf->insert(buf->end(), bytes, bytes + size);
		    },
		    &m_EncodedFrame,
		    static_cast<int>(width),
		    static_cast<int>(height),
		    3,
		    m_RgbScratch.data(),
		    100 // JPEG quality (0-100)
		);

		// Build and send header + image
		FrameStreamHeader hdr{};
		hdr.magic      = FRAME_STREAM_MAGIC;
		hdr.version    = FRAME_STREAM_VERSION;
		hdr.format     = FRAME_FORMAT_JPEG;
		hdr.width      = width;
		hdr.height     = height;
		hdr.imageSize  = static_cast<uint32_t>(m_EncodedFrame.size());
		hdr.frameIndex = m_FrameIndex++;

		if (!SendAll(sock, &hdr, sizeof(hdr)) ||
		    !SendAll(sock, m_EncodedFrame.data(), m_EncodedFrame.size()))
		{
			// Send failed — mark socket as gone; AcceptLoop will pick up the next client
			SOCKET expected = sock;
			if (m_ClientSocket.compare_exchange_strong(expected, INVALID_SOCKET, std::memory_order_acq_rel))
				closesocket(sock);
		}
	}

	void CloseClient()
	{
		SOCKET sock = m_ClientSocket.exchange(INVALID_SOCKET, std::memory_order_acq_rel);
		if (sock != INVALID_SOCKET)
			closesocket(sock);
	}

	bool SendAll(SOCKET sock, const void* data, size_t size)
	{
		const char* ptr  = static_cast<const char*>(data);
		size_t      left = size;
		while (left > 0)
		{
			int sent = ::send(sock, ptr, static_cast<int>(left), 0);
			if (sent <= 0)
				return false;
			ptr += sent;
			left -= sent;
		}
		return true;
	}

	// Pending frame handed off from the render thread to the worker (protected by m_WorkMutex)
	struct PendingFrame
	{
		std::vector<uint8_t> data;
		uint32_t             width    = 0;
		uint32_t             height   = 0;
		bool                 hasFrame = false;
	};

	SOCKET              m_ListenSocket = INVALID_SOCKET;
	std::atomic<SOCKET> m_ClientSocket{INVALID_SOCKET};
	std::atomic<bool>   m_Running{true};
	std::thread         m_AcceptThread;
	uint32_t            m_FrameIndex = 0u;

	// Worker thread state
	std::mutex              m_WorkMutex;
	std::condition_variable m_WorkCV;
	PendingFrame            m_Pending;
	std::thread             m_WorkerThread;

	// Encode buffers — accessed exclusively by the worker thread
	std::vector<uint8_t> m_RgbScratch;
	std::vector<uint8_t> m_EncodedFrame;
};
