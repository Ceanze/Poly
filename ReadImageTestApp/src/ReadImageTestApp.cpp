/**
 * ReadImageTestApp
 *
 * Connects to the RuntimeHost TCP frame stream and displays each received
 * frame in an ImGui window.
 *
 * Run RuntimeHost first (it listens on port 9100), then launch this app.
 * A background thread continuously receives FrameStreamHeader + JPEG data.
 * When a new frame arrives the main thread decodes it with stb_image and
 * uploads it to a GPU texture via ResourceLoader::LoadTextureFromMemory,
 * then displays it with ImGui::Image().
 */

#define WIN32_LEAN_AND_MEAN
#include "FrameStreamProtocol.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Rendering/RenderGraph/Passes/ImGuiPass.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Resources/ResourceLoader.h"

#include <imgui/imgui.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

// ---------------------------------------------------------------------------
// Layer
// ---------------------------------------------------------------------------

class ReadImageLayer : public Poly::Layer
{
public:
	ReadImageLayer() = default;

	void OnAttach() override
	{
		m_pGraph = Poly::RenderGraph::Create("ReadImageGraph");
		m_pGraph->AddPass(Poly::ImGuiPass::Create(), Poly::PassID("ImGuiPass"));
		m_pGraph->MarkOutput(Poly::PassResID("ImGuiPass", "fColor"));

		m_pProgram = m_pGraph->Compile();
		Poly::Application::Get().GetRenderer()->SetRenderGraph(m_pProgram);

		WSADATA wsaData = {};
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		m_Running    = true;
		m_RecvThread = std::thread(&ReadImageLayer::ReceiveLoop, this);
	}

	void OnDetach() override
	{
		m_Running = false;

		// Close the active socket to unblock any pending recv()
		SOCKET sock = m_Socket.exchange(INVALID_SOCKET, std::memory_order_acq_rel);
		if (sock != INVALID_SOCKET)
			closesocket(sock);

		if (m_RecvThread.joinable())
			m_RecvThread.join();

		WSACleanup();
	}

	void OnUpdate(Poly::Timestamp /*dt*/) override
	{
		// 1. Promote the back slot to front now that the previous upload is complete
		//    and the GPU has finished rendering the old front (LoadTextureFromMemory
		//    drains the queues before returning, so the swap is always safe).
		if (m_BackReady)
		{
			std::swap(m_FrontIdx, m_BackIdx);
			m_BackReady = false;
		}

		// 2. Grab the latest encoded frame — hold the lock only for the vector swap
		//    (microseconds), not for the decode or GPU upload.
		std::vector<uint8_t> encoded;
		uint32_t             netW = 0, netH = 0;
		{
			std::lock_guard<std::mutex> lock(m_FrameMutex);
			if (m_Pending.isNew)
			{
				encoded         = std::move(m_Pending.encodedData);
				netW            = m_Pending.width;
				netH            = m_Pending.height;
				m_Pending.isNew = false;
			}
		}

		// 3. Decode + GPU upload into the back slot — all outside the lock so the
		//    receive thread is never blocked by GPU work.
		if (!encoded.empty())
		{
			int   w      = 0, h = 0;
			byte* pixels = Poly::ResourceLoader::DecodeImageFromMemory(
			    encoded.data(), static_cast<int>(encoded.size()), &w, &h);

			if (pixels)
			{
				// Upload pixels into the back texture.
				m_pTextures[m_BackIdx] = Poly::ResourceLoader::LoadTextureFromMemory(
				    pixels, w, h, 4, Poly::EFormat::R8G8B8A8_UNORM);
				Poly::ResourceLoader::FreeDecodedImage(pixels);

				if (m_pTextures[m_BackIdx])
				{
					// Recreate the view to match the new texture object.
					Poly::TextureViewDesc viewDesc{};
					viewDesc.pTexture        = m_pTextures[m_BackIdx].get();
					viewDesc.ImageViewType   = Poly::EImageViewType::TYPE_2D;
					viewDesc.ImageViewFlag   = Poly::FImageViewFlag::SHADER_RESOURCE;
					viewDesc.Format          = Poly::EFormat::R8G8B8A8_UNORM;
					viewDesc.MipLevelCount   = 1;
					viewDesc.ArrayLayerCount = 1;
					m_pTextureViews[m_BackIdx] = Poly::RenderAPI::CreateTextureView(&viewDesc);

					m_TextureWidth  = static_cast<uint32_t>(w);
					m_TextureHeight = static_cast<uint32_t>(h);

					// Signal that the back slot is ready to be swapped next frame.
					m_BackReady = true;
				}
			}
		}

		// 4. Always render from the front slot — a fully completed, stable upload.
		ImGui::Begin("Streamed Frame Viewer");

		if (m_pTextureViews[m_FrontIdx] && m_TextureHeight > 0)
		{
			// Scale the image to fit the available width while preserving aspect ratio
			float avail  = ImGui::GetContentRegionAvail().x;
			float aspect = static_cast<float>(m_TextureWidth) / static_cast<float>(m_TextureHeight);
			ImGui::Image(reinterpret_cast<ImTextureID>(m_pTextureViews[m_FrontIdx].get()), ImVec2(avail, avail / aspect));
		}
		else
		{
			ImGui::TextColored({1.f, 0.4f, 0.4f, 1.f}, "Streamed frame not available.");
			ImGui::Text("Start RuntimeHost to start streaming frames.");
		}

		ImGui::End();
	}

private:
	// ---------------------------------------------------------------------------
	// Background receive thread
	// ---------------------------------------------------------------------------

	void ReceiveLoop()
	{
		while (m_Running)
		{
			SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sock == INVALID_SOCKET)
			{
				SleepInterruptible(500);
				continue;
			}

			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port   = htons(FRAME_STREAM_PORT);
			inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

			if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
			{
				closesocket(sock);
				SleepInterruptible(500);
				continue;
			}

			// Store socket so OnDetach() can close it to unblock recv()
			{
				SOCKET old = m_Socket.exchange(sock, std::memory_order_acq_rel);
				if (old != INVALID_SOCKET)
					closesocket(old);
			}

			// Receive frames until the connection drops or we're shutting down
			while (m_Running)
			{
				FrameStreamHeader hdr{};
				if (!RecvAll(sock, &hdr, sizeof(hdr)))
					break;

				if (hdr.magic != FRAME_STREAM_MAGIC || hdr.version != FRAME_STREAM_VERSION)
					break;

				std::vector<uint8_t> buf(hdr.imageSize);
				if (!RecvAll(sock, buf.data(), hdr.imageSize))
					break;

				std::lock_guard<std::mutex> lock(m_FrameMutex);
				m_Pending.encodedData = std::move(buf);
				m_Pending.width       = hdr.width;
				m_Pending.height      = hdr.height;
				m_Pending.frameIndex  = hdr.frameIndex;
				m_Pending.isNew       = true;
			}

			// Connection lost — clear socket and try to reconnect
			SOCKET expected = sock;
			if (m_Socket.compare_exchange_strong(expected, INVALID_SOCKET, std::memory_order_acq_rel))
				closesocket(sock);
		}
	}

	bool RecvAll(SOCKET sock, void* data, size_t size)
	{
		char*  ptr  = static_cast<char*>(data);
		size_t left = size;
		while (left > 0)
		{
			int received = ::recv(sock, ptr, static_cast<int>(left), 0);
			if (received <= 0)
				return false;
			ptr += received;
			left -= received;
		}
		return true;
	}

	// Sleep in small increments so we respond promptly to m_Running becoming false
	void SleepInterruptible(int totalMs)
	{
		for (int i = 0; i < totalMs / 50 && m_Running; ++i)
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// ---------------------------------------------------------------------------
	// Data
	// ---------------------------------------------------------------------------

	struct PendingFrame
	{
		std::vector<uint8_t> encodedData;
		uint32_t             width      = 0;
		uint32_t             height     = 0;
		uint32_t             frameIndex = 0;
		bool                 isNew      = false;
	};

	Poly::Ref<Poly::RenderGraph>        m_pGraph;
	Poly::Ref<Poly::RenderGraphProgram> m_pProgram;

	// TCP receive thread
	std::atomic<SOCKET> m_Socket{INVALID_SOCKET};
	std::atomic<bool>   m_Running{false};
	std::thread         m_RecvThread;

	// Frame double-buffer (receive thread writes, main thread reads)
	std::mutex   m_FrameMutex;
	PendingFrame m_Pending;

	// GPU double-buffer: front slot is displayed, back slot receives the next upload.
	// Indices are swapped at the top of OnUpdate once the back upload completes.
	Poly::Ref<Poly::Texture>     m_pTextures[2];
	Poly::Ref<Poly::TextureView> m_pTextureViews[2];
	uint32_t                     m_TextureWidth  = 0;
	uint32_t                     m_TextureHeight = 0;
	int                          m_FrontIdx      = 0;
	int                          m_BackIdx       = 1;
	bool                         m_BackReady     = false;
};

// ---------------------------------------------------------------------------
// Application
// ---------------------------------------------------------------------------

class ReadImageTestApp : public Poly::Application
{
public:
	void OnInit() override { PushLayer(new ReadImageLayer()); }

private:
	std::optional<Poly::Window::Properties> GetWindowProperties() const override
	{
		return Poly::Window::Properties{1280, 720, "Poly Frame Stream Viewer"};
	}
};

Poly::Application* Poly::CreateApplication()
{
	return new ReadImageTestApp();
}
