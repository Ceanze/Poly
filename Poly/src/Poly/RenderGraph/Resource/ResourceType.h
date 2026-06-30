#pragma once

namespace Poly
{
	enum class EResourceType
	{
		None,
		SampledImage,           // Texture_SRV  - read-only texture (sampled)
		StorageImage,           // Texture_UAV  - read/write texture
		UniformTexelBuffer,     // TypedBuffer_SRV  - read-only typed buffer
		StorageTexelBuffer,     // TypedBuffer_UAV  - read/write typed buffer
		StorageBuffer,          // StructuredBuffer_SRV  - read-only structured buffer
		StorageBufferReadWrite, // StructuredBuffer_UAV  - read/write structured buffer
		RawBuffer,              // RawBuffer_SRV  - read-only byte-addressable buffer
		RawBufferReadWrite,     // RawBuffer_UAV  - read/write byte-addressable buffer
		UniformBuffer,          // ConstantBuffer
		DynamicUniformBuffer,   // VolatileConstantBuffer
		Sampler,
		AccelerationStructure, // RayTracingAccelStruct
		PushConstants,
		SamplerFeedback, // SamplerFeedbackTexture_UAV
	};
}
