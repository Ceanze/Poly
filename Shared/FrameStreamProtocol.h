#pragma once

#include <cstdint>

static constexpr uint32_t FRAME_STREAM_MAGIC   = 0x504F4C59u; // 'POLY'
static constexpr uint16_t FRAME_STREAM_VERSION = 1u;
static constexpr uint16_t FRAME_FORMAT_JPEG    = 1u;
static constexpr uint16_t FRAME_FORMAT_PNG     = 2u;
static constexpr uint16_t FRAME_STREAM_PORT    = 9100u;

#pragma pack(push, 1)
struct FrameStreamHeader
{
	uint32_t magic;      // FRAME_STREAM_MAGIC
	uint16_t version;    // FRAME_STREAM_VERSION
	uint16_t format;     // FRAME_FORMAT_JPEG or FRAME_FORMAT_PNG
	uint32_t width;      // pixels
	uint32_t height;     // pixels
	uint32_t imageSize;  // byte count of encoded image data that follows this header
	uint32_t frameIndex; // monotonically increasing frame counter
};
// 24 bytes total
#pragma pack(pop)
