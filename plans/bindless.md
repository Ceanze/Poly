# Descriptor Indexing & Buffer Device Address (bindless)

## Context
The new extension `VK_EXT_Descriptor_Heaps` is not ready to be used yet as of writing (2026-07-19), mainly due to not having MoltenVK support.

Therefore, the older alternative of using descriptor indexing and Buffer Device Address (BDA) has to be used.

## Outline
1. Shader always has the global `g_Textures` and `g_Samplers` which uses descriptor indexing. These are set by the engine to always be set 0 with binding 0, 1.
2. Buffers, all of them, use BDA.
   1. To access a buffer, the user uses a `buffer_reference` type buffer, `layout(buffer_reference, std430) readonly buffer ParticleBuffer`
   2. The buffer is then created using `ParticleBuffer(slot[0])`, where `slot[0]` is a uint64 buffer address. Slot is provided by push constant (initially) or a (normal) uniform/SSBO buffer.
3. The push constant holds two fixed-size arrays, shared across every shader via the same header: `uint textureIndices[16]` (descriptor-indexing indices into `g_Textures`/`g_Samplers`) and `uint64 bufferAddresses[16]` (BDA pointers). Since the arrays live at the same offsets in every shader, reflection's job is just to confirm the layout is present and report those offsets - it does not (yet) know which array *slot* a given resource belongs to.
4. Resources are setup from the render graph as usual `Update/SetResource("Resource", resourceBuffer);`
5. Only the shader hard-codes a literal slot index today (`bufferAddresses[0]`) - this is temporary and will go away once shaders move to Slang. The client side never specifies a slot number: it still calls `ImportResource("Resource", "shaderName")` as usual, and the engine assigns the actual slot internally, in the order resources are imported. The two sides stay in sync only by matching declaration order, which is exactly what Slang generation is meant to remove the need for later.

## Code example

### Shader side

Every bindless-consuming shader includes one shared header. It declares the global texture/sampler heap
*and* the two fixed-size slot arrays that every shader's push constant starts with - same names, same
order, same offsets in every shader, engine-owned:

```glsl
// assets/shaders/common/bindless.glsl
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2    : require

#define BINDLESS_MAX_SLOTS 16

// Always set 0, binding 0/1 - identical across every shader, engine-owned.
layout(set = 0, binding = 0) uniform texture2D g_Textures[];
layout(set = 0, binding = 1) uniform sampler   g_Samplers[];

// Every push-constant block that wants bindless access starts with this macro so the two
// arrays land at the same offsets (0 and 64) in every shader that uses them.
#define BINDLESS_PUSH_CONSTANTS \
	uint     textureIndices[BINDLESS_MAX_SLOTS]; \
	uint64_t bufferAddresses[BINDLESS_MAX_SLOTS]
```

`textureIndices` holds indices into `g_Textures`/`g_Samplers` (regular descriptor indexing);
`bufferAddresses` holds BDA pointers, cast to a `buffer_reference` type at the point of use. Both are
indexed with a literal the shader author picks by hand - there's no name to look up yet:

```glsl
// assets/shaders/particles/particles.vert
#version 460
#include "common/bindless.glsl"

layout(buffer_reference, std430) readonly buffer ParticleBuffer
{
	vec4 data[]; // xyz = world position, w = size
};

layout(buffer_reference, std430) readonly buffer CameraBuffer
{
	mat4 viewProj;
};

layout(push_constant, std430) uniform PushConstants
{
	BINDLESS_PUSH_CONSTANTS;
} params;

layout(location = 0) out vec2 out_UV;

void main()
{
	// Slot 0/1 are hard-coded here and must match whatever slot the pass declaration
	// (client side, below) assigns "particles.Buffer" / "$.scene:Camera" to.
	vec4 posSize = ParticleBuffer(params.bufferAddresses[0]).data[gl_InstanceIndex];
	mat4 viewProj = CameraBuffer(params.bufferAddresses[1]).viewProj;

	gl_Position = viewProj * vec4(posSize.xyz, 1.0);
	out_UV      = vec2(0.0); // quad expansion omitted
}
```

```glsl
// assets/shaders/particles/particles.frag
#include "common/bindless.glsl"

layout(push_constant, std430) uniform PushConstants
{
	BINDLESS_PUSH_CONSTANTS;
} params;

layout(location = 0) in  vec2 in_UV;
layout(location = 0) out vec4 out_Color;

void main()
{
	// Slot 0 here is a *texture* slot, unrelated to the buffer slot 0 used in the vertex shader -
	// textureIndices and bufferAddresses are two independent arrays.
	out_Color = texture(sampler2D(g_Textures[params.textureIndices[0]], g_Samplers[0]), in_UV);
}
```

Sixteen slots of each isn't free: `textureIndices[16]` (64 bytes) + `bufferAddresses[16]` (128 bytes) is
192 bytes already, close to the Vulkan-guaranteed minimum `maxPushConstantsSize` of 128 bytes (most
desktop drivers allow 256, but it isn't spec-guaranteed). A pass that needs more slots than fit is the
"or a normal uniform/SSBO buffer" fallback from the outline - the array layout and slot-matching mechanics
below stay the same either way, only where the bytes get uploaded from changes.

### Reflection: confirming the fixed layout, not deriving the mapping

Because `textureIndices`/`bufferAddresses` sit at the same fixed offsets in every shader (they come from
one shared macro), reflection doesn't need to discover *which* resource a slot belongs to - it only needs
to confirm the arrays are present and report where they start, so the engine knows where to write into the
push-constant bytes:

```cpp
// Poly/src/Poly/Resources/Shader/ShaderReflection.h
struct ShaderBindlessLayout
{
	bool   HasTextureSlots    = false;
	uint32 TextureSlotsOffset = 0; // byte offset of textureIndices[0]
	uint32 TextureSlotCount   = 0; // BINDLESS_MAX_SLOTS, read off the array's reflected element count

	bool   HasBufferSlots     = false;
	uint32 BufferSlotsOffset  = 0; // byte offset of bufferAddresses[0]
	uint32 BufferSlotCount    = 0;
};

struct ShaderReflection
{
	std::vector<ShaderInputOutput>  Inputs;
	std::vector<ShaderInputOutput>  Outputs;
	std::vector<ShaderBinding>      Bindings;
	std::vector<ShaderPushConstant> PushConstants;
	ShaderBindlessLayout            BindlessLayout; // NEW - offsets of textureIndices[]/bufferAddresses[], if present
};
```

```cpp
// Poly/src/Poly/Resources/Shader/ShaderReflector.cpp
ShaderReflection ShaderReflector::Reflect()
{
	ShaderReflection reflection;
	// ... existing Inputs/Outputs/Bindings/PushConstants extraction ...

	for (const SpvReflectBlockVariable& block : m_Module->push_constant_blocks)
	{
		for (const SpvReflectBlockVariable& member : block.members)
		{
			if (std::string_view(member.name) == "textureIndices")
			{
				reflection.BindlessLayout.HasTextureSlots    = true;
				reflection.BindlessLayout.TextureSlotsOffset = member.offset;
				reflection.BindlessLayout.TextureSlotCount   = member.array.dims[0];
			}
			else if (std::string_view(member.name) == "bufferAddresses")
			{
				reflection.BindlessLayout.HasBufferSlots     = true;
				reflection.BindlessLayout.BufferSlotsOffset  = member.offset;
				reflection.BindlessLayout.BufferSlotCount    = member.array.dims[0];
			}
		}
	}
	return reflection;
}
```

### Pass declaration: mapping render graph resources (client side)

The client side never deals with numeric slots at all - it keeps using the same `ImportResource(resourceName,
shaderResourceName)` call as every other pass resource. `shaderResourceName` is now just a human-readable
label (there's nothing left in the shader to look it up against); the actual slot number is assigned
internally by `RenderProgramBuilder`, not by the caller:

```cpp
// wherever passes are registered, e.g. Poly/src/Poly/Rendering/Passes/ParticlesPass.cpp
catalog.RegisterPass("particles")
    .WithShader("shaders/particles/particles.vert", FShaderStage::VERTEX)
    .WithShader("shaders/particles/particles.frag", FShaderStage::FRAGMENT)
    .ImportResource("particles.Buffer", "particles") // buffer-shaped -> next free bufferAddresses[] slot
    .ImportResource("$.scene:Camera",   "camera")    // buffer-shaped -> next free bufferAddresses[] slot
    .ImportResource("particles.Albedo", "albedo")    // texture-shaped -> next free textureIndices[] slot
    .WithGraphicsPipeline()
        .AddVertexInput()
            // ... unrelated to bindless, quad/instance layout as usual ...
        .FinishVertexInput()
        .FinishPipeline()
    .WithExecuteFn([](ExecuteContext& ctx)
    {
        ctx.GetCommandBuffer()->DrawIndirect(...);
    });
```

At compile time, `RenderProgramBuilder` walks a pass's `ImportResource` calls in declaration order. For
each one, it looks up the matching `ResourceDeclaration::GetResourceType()` and assigns the next free
index into `BufferSlots` (buffer-shaped types) or `TextureSlots` (`SampledImage`) - the caller never picks
a number. This is also the one place the outline's "hand-sync" problem still shows up: the *order* these
calls appear in has to line up with the order the shader author indexed `bufferAddresses`/`textureIndices`
with literals. `RenderProgramBuilder` can bounds-check the assigned indices against the reflected
`BindlessLayout` (`< BufferSlotCount`/`< TextureSlotCount`), but it can't verify slot 0 is semantically
`particles.Buffer` and not `$.scene:Camera` - that's exactly the coupling Slang is meant to remove later,
once the shader stops hard-coding literal indices at all.

```cpp
// Poly/src/Poly/RenderGraph/RenderProgram.h
struct ResolvedSlot
{
	uint32      Slot;         // index into textureIndices[] or bufferAddresses[]
	std::string ResourceName; // render-graph resource providing the value, e.g. "particles.Buffer"
};

struct ResolvedPass
{
	std::string               Name;
	std::vector<ResolvedPort> Ports;

	std::vector<ResolvedSlot> BufferSlots;        // NEW - buffer-shaped ImportResource()s, assigned + bounds-checked here
	std::vector<ResolvedSlot> TextureSlots;       // NEW - texture-shaped ImportResource()s, assigned here
	uint32                    BufferSlotsOffset  = 0; // NEW - from ShaderBindlessLayout
	uint32                    TextureSlotsOffset = 0; // NEW - from ShaderBindlessLayout
	uint32                    PushConstantSize   = 0; // NEW - total push-constant block size, from reflection

	std::vector<std::pair<std::string, FShaderStage>> Shaders;
	GraphicsPipelineDesc                              PipelineDesc;
	std::function<void(ExecuteContext&)>              ExecuteFn;

	FQueueType Queue = FQueueType::GRAPHICS;
};
```

### Runtime: resolving slots to device addresses / heap indices

Resources are still supplied the normal way, once per frame:

```cpp
// SandboxApp.cpp
m_pProgramInstance->UpdateResource("particles.Buffer",  m_pParticleBuffer);
m_pProgramInstance->UpdateResource("$.scene:Camera",    m_pCameraBuffer);
m_pProgramInstance->UpdateResource("particles.Albedo",  m_pAlbedoTexture);
```

`RenderProgramInstance::Execute()` resolves each pass's `BufferSlots`/`TextureSlots` into device
addresses/heap indices and packs them into the push-constant bytes right before the pass's `ExecuteFn`
runs:

```cpp
// Poly/src/Poly/RenderGraph/RenderProgramInstance.cpp
void RenderProgramInstance::Execute(const RenderView& view)
{
	CommandBuffer* pCmdBuffer = /* ... begin frame command buffer ... */;

	for (const ResolvedPass& pass : m_pRenderProgram->GetPasses())
	{
		// ... bind pipeline, framebuffer, begin render pass (unrelated to bindless) ...

		if (pass.PushConstantSize > 0)
		{
			std::vector<byte> pushData(pass.PushConstantSize, 0);

			for (const ResolvedSlot& slot : pass.BufferSlots)
			{
				const Buffer* pBuffer = m_ResourceCache.GetBuffer(slot.ResourceName);
				uint64        address = pBuffer->GetDeviceAddress();
				uint32        offset  = pass.BufferSlotsOffset + slot.Slot * sizeof(uint64);
				std::memcpy(pushData.data() + offset, &address, sizeof(uint64));
			}

			for (const ResolvedSlot& slot : pass.TextureSlots)
			{
				uint32 heapIndex = m_ResourceCache.GetTextureHeapIndex(slot.ResourceName);
				uint32 offset    = pass.TextureSlotsOffset + slot.Slot * sizeof(uint32);
				std::memcpy(pushData.data() + offset, &heapIndex, sizeof(uint32));
			}

			pCmdBuffer->UpdatePushConstants(pPipelineLayout, FShaderStage::VERTEX | FShaderStage::FRAGMENT, 0, (uint32)pushData.size(), pushData.data());
		}

		ExecuteContext ctx(pCmdBuffer, view);
		pass.ExecuteFn(ctx);
	}
}
```

`Buffer::GetDeviceAddress()` is new (`Platform/API/Buffer.h`), backed by `vkGetBufferDeviceAddress` in
`PVKBuffer`, requiring buffers to opt in via a new `FBufferUsage::SHADER_DEVICE_ADDRESS` flag so only
buffers actually consumed through `buffer_reference` pay for the `VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT`
+ `VkBufferDeviceAddressInfo` query cost. `m_ResourceCache.GetTextureHeapIndex()` is the texture-heap
counterpart to `GetDeviceAddress()` - it returns whatever index the texture was registered at in the
`g_Textures`/`g_Samplers` heap (registration itself, e.g. at asset-load time, is out of scope for this doc).

## Future work

- Auto-generate the `MapBufferSlot`/`MapTextureSlot` indices and the matching shader-side literals from a
  single source of truth instead of hand-syncing them, either via a custom preprocessing step over the
  GLSL source or by moving shaders to Slang, which has native support for this kind of binding generation.