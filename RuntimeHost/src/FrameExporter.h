#pragma once

#include <cstdint>

/**
 * Abstract interface for exporting rendered frames out of the RuntimeHost process.
 *
 * Implementations decide HOW the pixel data is transported (shared memory,
 * WebSocket, disk file, …). The caller provides raw BGRA8 pixels together
 * with their dimensions and the implementation handles serialisation and
 * transport.
 *
 * Call WriteFrame once per rendered frame, after CopyData has been called on
 * the ReadTexturePass staging buffer (i.e. after the GPU fence for the
 * previous frame has been signalled).
 */
class FrameExporter
{
public:
	virtual ~FrameExporter() = default;

	/**
	 * Export one frame.
	 * @param pData    Raw BGRA8 pixel data (row-major, top-to-bottom).
	 * @param dataSize Total byte count of pData (== width * height * 4).
	 * @param width    Image width in pixels.
	 * @param height   Image height in pixels.
	 */
	virtual void WriteFrame(const void* pData, uint64_t dataSize, uint32_t width, uint32_t height) = 0;
};
