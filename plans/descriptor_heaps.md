# Descriptor-Heap (Bindless) Migration for the New Render Graph

## Context

The new render graph (`Poly/src/Poly/RenderGraph/`) is a from-scratch rewrite of Poly's rendering
architecture, living alongside (and eventually replacing) the legacy `Poly/src/Poly/Rendering/RenderGraph/`.
Its design doc (`plans/render_graph.md`) lists "Bindless — use the new bindless option in Vulkan" as
requirement #2 and explicitly flags it as unstarted research. The compile-time half of the new graph
(`RenderCatalog`, `RenderProgramBuilder`, `RenderProgram`, cross-queue `SyncPlan`) is largely built. The
runtime half, `RenderProgramInstance::Execute()`, is a **complete no-op stub** — nothing allocates GPU
resources, binds descriptors, or dispatches draws yet. This is deliberate: the descriptor model for the
new graph can be designed clean, with no legacy per-pass `DescriptorCache` to unwind.

The directive: the new graph supports **only** descriptor heaps, targeting the real Vulkan extension
**`VK_EXT_descriptor_heap`** (confirmed via `docs.vulkan.org` — this extension exists but is very new; it
is not necessarily present in the Vulkan SDK/loader/driver currently installed locally, and **Poly's
vendored `glslang` is v13.0.0 from 2023-08-23, which has no support for this extension's SPIR-V
capability at all** — see Prerequisites below). Per your direction, the plan targets this extension
directly rather than building an interim backend on something else.

`VK_EXT_descriptor_heap` was designed specifically to fix problems in `VK_EXT_descriptor_buffer` by
imposing stricter, more portable guarantees: **exactly one resource heap and one sampler heap per
device** (not an arbitrary number of descriptor buffers), and it **eliminates descriptor sets and
pipeline layouts entirely** for heap-bound resources — descriptors are looked up purely by heap offset.
This is a stronger match for the "one ResourceHeap" requirement than any prior draft of this plan: the
one-heap-per-device constraint isn't just a design choice here, it's literally spec-mandated.

Confirmed API surface (from the extension's reference page):
- **Heap binding:** `vkCmdBindResourceHeapEXT`, `vkCmdBindSamplerHeapEXT` (+ `VkBindHeapInfoEXT`) — bind
  the (one) resource heap and the (one) sampler heap to a command buffer.
- **Populating descriptors:** `vkWriteResourceDescriptorsEXT`, `vkWriteSamplerDescriptorsEXT` (+
  `VkResourceDescriptorInfoEXT`/`VkImageDescriptorInfoEXT`/`VkTexelBufferDescriptorInfoEXT`) — write a
  descriptor directly into a specific heap offset. No `VkDescriptorPool`, no `VkDescriptorSet`, no
  `vkUpdateDescriptorSets`.
- **Push data:** `vkCmdPushDataEXT` (+ `VkPushDataInfoEXT`) — a new mechanism replacing push constants,
  explicitly designed to eliminate the need to declare a push-constant range in a pipeline layout at
  pipeline-creation time.
- **Sizing:** `vkGetPhysicalDeviceDescriptorSizeEXT` — query the byte size of a descriptor for a given
  resource kind, needed to size the heaps and compute per-slot offsets.
- **Legacy shader compatibility:** `VkDescriptorSetAndBindingMappingEXT` /
  `VkShaderDescriptorSetAndBindingMappingInfoEXT` — lets a shader still written with classic
  `layout(set=, binding=)` GLSL decorations be mapped onto heap offsets at pipeline-creation time,
  without the shader itself needing to know about heaps at all.
- **Features/properties:** `VkPhysicalDeviceDescriptorHeapFeaturesEXT` /
  `VkPhysicalDeviceDescriptorHeapPropertiesEXT`.
- Native (non-mapped) shader access requires the `SPV_EXT_descriptor_heap` SPIR-V capability.

**Two shader-authoring paths this extension supports — this plan targets the first:**
1. **Mapping-compatibility path (recommended primary target):** shaders keep writing ordinary
   `layout(set=, binding=)` GLSL — something Poly's current, unmodified `glslang` v13.0.0 already fully
   supports — and `VkDescriptorSetAndBindingMappingEXT` remaps those decorations onto heap offsets at
   pipeline-creation time. This satisfies "no shader author specifies set/binding for the heap itself"
   (see design decisions below: the decorations live once in a shared header, never per-shader) without
   requiring any `glslang` upgrade.
2. **Native path (future upgrade, not blocking this plan):** shaders drop set/binding decorations
   entirely and address the heap directly via offsets, requiring `SPV_EXT_descriptor_heap` support in
   the SPIR-V generator. This plan does not assume exact GLSL syntax for this path since it isn't
   published anywhere yet — treat it as a drop-in future upgrade once `glslang` ships support, behind the
   same C++ `DescriptorHeap` abstraction.

## Prerequisites (must be resolved before implementation starts)

1. **Vulkan SDK / loader / driver** must support `VK_EXT_descriptor_heap`. Confirm `vkEnumerateDeviceExtensionProperties`
   lists it for your target GPU/driver before writing any Vulkan-layer code.
2. **`Poly/libs/glslang`** is pinned at v13.0.0 (2023-08-23) and has zero knowledge of this extension.
   This does **not** block the mapping-compatibility shader path (path 1 above only needs ordinary GLSL,
   which v13.0.0 already compiles fine) — it only blocks the native path (path 2). No `glslang` upgrade is
   required to implement this plan as scoped.
3. Confirm the Vulkan C headers vendored/referenced by `Poly/src/Platform/Vulkan/` (via the `VULKAN_SDK`
   environment variable per `CLAUDE.md`) declare the `VK_EXT_descriptor_heap` types/functions — if the
   installed SDK predates the extension's headers, the SDK itself needs updating first, independent of
   `glslang`.

## Key design decisions

**Heap ownership (the user's "not a static class" constraint).** Texture/buffer heap indices are
assigned at asset-load time via `ResourceManager` and must stay valid across render-graph rebuilds —
`RenderProgramInstance` is replaced wholesale whenever the graph recompiles (`render_graph.md`'s "safe
point to switch instance"), so the heap cannot live inside it. It also cannot be a bare static singleton
(the exact pattern being avoided). The fix: one `DescriptorHeap` instance, constructed once by `Renderer`
right after device creation, with a single explicit owner (`Renderer`) and explicit lifetime.
`ResourceManager` and every `RenderProgramInstance` receive a **borrowed pointer** to it rather than
reaching it through hidden global state.

**Heap organization: exactly one resource heap + one sampler heap (spec-mandated), with typed offset
ranges inside the resource heap.** Since the extension itself only allows one resource heap and one
sampler heap per device, "ResourceHeap" and "TextureHeap" in earlier drafts of this plan collapse into
**one** resource heap covering textures *and* every buffer shape (vertices, instances, materials, camera,
lights, draw data); the sampler heap stays conceptually separate only because the extension itself
requires it. `DescriptorHeap::RegisterTexture`/`RegisterBuffer` both allocate slots from the same
underlying resource heap, just written via `vkWriteResourceDescriptorsEXT` with different
`VkResourceDescriptorInfoEXT`/`VkImageDescriptorInfoEXT` payloads depending on kind — the heap itself
doesn't track "shape", only that a slot is occupied.

**Simplification this implies:** `Camera` moves from a true uniform buffer (`uniform Camera {...}`) to a
read-only storage buffer (`buffer Camera {...}`) for consistency with how the mapping-compatibility path
declares heap-backed buffers. Functionally unchanged (still read-only, same data), just a different
descriptor kind — flagged below for confirmation.

**Avoiding manual `set`/`binding` authoring.** All heap-related `layout(set=X, binding=Y)` declarations
live in exactly one shared GLSL header, `assets/shaders/common/bindless_heap.glsl`, `#include`d by every
heap-consuming shader (`ShaderCompiler.cpp:49-54` already wires a glslang `DirStackFileIncluder` before
parsing, so `#include` already works — no new tooling needed). Individual shader files and
`PassDeclaration` authors never write a `layout(set=,binding=)` line or configure a heap binding
themselves; on the C++ side, every heap pass shares one `VkShaderDescriptorSetAndBindingMappingInfoEXT`
mapping (built once by `DescriptorHeap`, not per pass) that remaps the header's fixed decorations onto
the real heap offsets at pipeline-creation time.

**Indirection: one push-data value, `uint DrawID`, via `vkCmdPushDataEXT` (not classic push constants).**
`DrawID` indexes a `DrawData` entry (itself heap-resident) carrying the resource-heap indices for every
buffer that draw needs — vertices, instances, materials (+ element), camera, lights. `MaterialValues`
grows six texture heap-index fields so the already-existing per-material indexing (`in_MaterialIndex` →
`MaterialProperties`) carries bindless texture indices with no separate lookup table.

**Correction from earlier drafts:** `CommandBuffer::UpdatePushConstants`/`PipelineLayout`'s
`PushConstantRange` are the *classic* push-constant mechanism and are **not** reused here — `vkCmdPushDataEXT`
is a distinct, new API specifically designed to avoid needing a pipeline-layout-declared range at all.
Add a new `CommandBuffer::PushData(const void* data, uint32 size)` abstraction method instead (Vulkan impl
wraps `vkCmdPushDataEXT`/`VkPushDataInfoEXT`).

## Changes by area

### 1. Vulkan device/API layer

- **`Poly/src/Platform/Vulkan/PVKInstance.cpp` (~line 601, logical device creation):**
  - Add `VkPhysicalDeviceDescriptorHeapFeaturesEXT` to the feature `pNext` chain (alongside the existing
    `Vulkan12Features`/`Vulkan13Features`), enabling the extension's core feature bit(s). **Exact field
    name(s) need confirming against the actual SDK header once installed** — treat this struct's field
    list as unconfirmed pending your SDK update.
  - Add device extension `VK_EXT_descriptor_heap`'s extension-name macro to the extension list
    (`PVKInstance.cpp:682-686` area) alongside the existing conditional extensions.
  - Query `VkPhysicalDeviceDescriptorHeapPropertiesEXT` for heap size/alignment limits before sizing the
    heap buffers.
- **New abstraction, `Poly/src/Platform/API/DescriptorHeap.h`:** does not extend `DescriptorSet` (wrong
  model entirely — no pools, no `VkDescriptorSet`/`VkPipelineLayout` objects for heap-bound resources at
  all). New interface:
  - `Init(const DescriptorHeapDesc*)`
  - `RegisterTexture(const TextureView*, ETextureLayout) -> uint32` / `UpdateTexture(...)` /
    `ReleaseTexture(uint32)` — bump-allocator + free-list over the resource heap's image-descriptor
    range.
  - `RegisterSampler(const Sampler*) -> uint32` / `ReleaseSampler(uint32)` — bump-allocator over the
    (separate, spec-mandated) sampler heap. A default sampler is registered once at heap-init time at a
    well-known index (e.g. 0) and used by materials unless/until per-texture custom samplers are needed.
  - `RegisterBuffer(const Buffer*, uint64 offset, uint64 range) -> uint32` / `UpdateBuffer(...)` /
    `ReleaseBuffer(uint32)` — one bump-allocator + free-list over the resource heap's buffer-descriptor
    range (shared across every buffer shape — vertices, instances, materials, camera, lights, draw data).
  - `BindHeaps(CommandBuffer*)` — issues `vkCmdBindResourceHeapEXT` + `vkCmdBindSamplerHeapEXT` once per
    frame.
  - `GetShaderMapping() const` — the one `VkShaderDescriptorSetAndBindingMappingInfoEXT`-based mapping
    object every heap pipeline is created with, remapping the shared header's fixed
    `layout(set=,binding=)` decorations onto real heap offsets.
- **`Poly/src/Platform/Vulkan/PVKDescriptorHeap.h/.cpp`:**
  - Two backing allocations (resource heap, sampler heap), each created per the extension's heap-buffer
    creation requirements, sized via `vkGetPhysicalDeviceDescriptorSizeEXT`/heap property limits.
  - `RegisterX` calls `vkWriteResourceDescriptorsEXT`/`vkWriteSamplerDescriptorsEXT` directly — no manual
    `memcpy` of raw descriptor bytes needed (unlike `VK_EXT_descriptor_buffer`'s `vkGetDescriptorEXT`
    pattern from an earlier draft of this plan — this extension's write calls populate the heap for you).
  - `BindHeaps(CommandBuffer*)` calls `vkCmdBindResourceHeapEXT` + `vkCmdBindSamplerHeapEXT` (via
    `VkBindHeapInfoEXT`) once per command buffer per frame — no per-pass, no per-pipeline rebinding.
  - Builds the `VkDescriptorSetAndBindingMappingEXT`/`VkShaderDescriptorSetAndBindingMappingInfoEXT` once,
    mapping each `(set, binding)` pair used in `bindless_heap.glsl` to its corresponding heap offset range,
    reused unchanged by every heap pipeline's creation.
  - **Needs verification against the actual spec/headers once your SDK is updated:** whether graphics
    pipeline creation for heap-mapped shaders still requires a (possibly trivial/empty) `VkPipelineLayout`
    handle structurally, or whether the mapping info structs supersede it entirely. Don't guess this at
    implementation time — check the header/spec directly.
- **`RenderAPI::CreateDescriptorHeap(const DescriptorHeapDesc*)`** — factory method alongside the
  existing `CreateTextureView` etc.
- **`Poly/src/Platform/API/CommandBuffer.h`:** add `virtual void PushData(const void* pData, uint32 size) = 0;`
  alongside (not replacing) the existing `UpdatePushConstants` — heap pipelines use `PushData`; any
  remaining non-heap/legacy pipelines keep using classic push constants.

### 2. Asset loading & material representation

- **`Poly/src/Poly/Resources/ResourceTypes.h`:** `ManagedTexture` gains `uint32 HeapIndex = UINT32_MAX`.
- **`Poly/src/Poly/Resources/ResourceManager.h/.cpp`:** add `static void SetDescriptorHeap(DescriptorHeap*)`
  (borrowed pointer, set once by `Renderer` before `ResourceManager::Init()` runs) and
  `static uint32 GetTextureHeapIndex(PolyID)`. In `LoadTexture` (`ResourceManager.cpp:53-89`), right
  after the `TextureView` is created and pushed into `m_Textures`, call `RegisterTexture` and store the
  returned index on that `ManagedTexture`. Same insertion in `RegisterDefaultMaterial`
  (`ResourceManager.cpp:338-369`) so the default white texture gets a valid index (used as every
  `MaterialValues::*Tex` default).
- **`Poly/src/Poly/Model/Material.h`:** `MaterialValues` grows six `uint32` fields (`AlbedoTex`,
  `MetallicTex`, `NormalTex`, `RoughnessTex`, `AOTex`, `CombinedTex`, defaulting to the registered
  default-texture index) plus two `uint32` pad fields to keep std430 alignment clean (32 → 64 bytes).
  Populate them wherever a `Material`'s textures are currently set (`Material::SetTexture`/glTF import
  path) by reading each texture's `ManagedTexture.HeapIndex` — `Material` keeps its existing
  `Texture*`/`TextureView*` maps (harmless, still useful for CPU-side queries), but the GPU-visible
  struct now also carries heap indices.
- **`Poly/src/Poly/Model/Mesh.h`:** cache `uint32 m_VertexHeapIndex = UINT32_MAX`; add a lazy accessor
  that registers the mesh's vertex buffer into the resource heap on first use. Index buffers stay
  conventionally bound (`BindIndexBuffer`) — they are not heap resources.

### 3. New RenderGraph runtime (`RenderProgramInstance`)

- **Per-frame scene upload:** replace the per-batch descriptor-set-3 rebind in
  `RenderScene::Update()`/`Execute()` (`RenderScene.cpp:20-99`) with a heap-aware path (new type, e.g.
  `HeapSceneUploader`, under `Poly/src/Poly/RenderGraph/`) that, per batch: gets the mesh's vertex heap
  index, registers/updates the batch's instance-transform buffer and material buffer into the resource
  heap, and appends one `DrawData` entry (`{Vertices, Instances, Materials, MaterialElem, Camera, Lights}`,
  all `uint32` heap indices, matching the GLSL struct 1:1) to a per-frame `DrawData` buffer, itself also
  registered into the heap. Camera and lights are registered once per frame, not per batch.
- **`RenderProgramInstance.h/.cpp`:** constructor gains a borrowed `DescriptorHeap*`. `Execute(view)`
  stops being a no-op:
  1. Lazily (first call) build per-`ResolvedPass` `RenderPass`/`Framebuffer`/`Pipeline` objects, every
     pipeline created using `m_pHeap->GetShaderMapping()` (see prerequisite note on whether a trivial
     `VkPipelineLayout` is still additionally required); allocate transient resources for `ResolvedPort`s
     with `IsExternal == false`.
  2. Run the heap scene upload for this frame.
  3. `m_pHeap->BindHeaps(pCmdBuffer)` **once per frame** — binds the resource heap and sampler heap; no
     further per-pass or per-pipeline descriptor binding calls are needed for the rest of the frame.
  4. Per pass in `RenderProgram::GetPasses()` order: apply that pass's `SyncPlan` `PreBarriers`/
     `Acquires` (already computed by `RenderProgramBuilder::PlanSynchronization`), begin its render pass,
     bind its pipeline, call `ExecuteFn(ExecuteContext)`, end the render pass, apply `PostReleases`.
     **Confirmed:** extend `Poly/src/Poly/RenderGraph/Resource/ResourceUsage.cpp`'s `DeriveResourceUsage`
     so `SampledImage`/`StorageBuffer` (and other heap-resident types) keep full barrier tracking exactly
     as they do today — heap-resident resources still need real layout transitions and read/write
     barriers even though binding is bindless. Only `Sampler`, `PushConstants`, `AccelerationStructure`,
     and `SamplerFeedback` remain genuinely untracked (unchanged from today).
  5. Inside each pass's `ExecuteFn`: per scene batch, `PushData(&drawID, sizeof(uint32))` then
     `BindIndexBuffer` + `DrawIndexedInstanced` — no vertex-buffer bind (vertex pulling through the heap),
     no per-batch descriptor rebind.
- **`Poly/src/Poly/Rendering/Renderer.h/.cpp`:** own `std::unique_ptr<DescriptorHeap> m_pGlobalHeap`,
  constructed right after device init and before `ResourceManager::Init()`; call
  `ResourceManager::SetDescriptorHeap(m_pGlobalHeap.get())`; pass the same raw pointer into every
  `RenderProgramInstance` created by `SetRenderProgram`.

### 4. Shader authoring — shared header + new `pbr_heap.vert`/`pbr_heap.frag`

Add `assets/shaders/common/bindless_heap.glsl` (new shared include — the **only** file with heap
`layout(set=,binding=)` declarations, which get remapped onto real heap offsets by
`VkDescriptorSetAndBindingMappingEXT` at pipeline-creation time rather than bound via a real descriptor
set):

```glsl
struct Vertex         { vec4 Position; vec4 Normal; vec4 Tangent; vec4 TexCoord; };
struct MaterialValues { vec4 Albedo; float AO, Metallic, Roughness, IsCombined;
                        uint AlbedoTex, MetallicTex, NormalTex, RoughnessTex, AOTex, CombinedTex, _p0, _p1; };
struct PointLight     { vec4 Color; vec4 Position; };
struct DrawData       { uint Vertices, Instances, Materials, MaterialElem, Camera, Lights, _p0, _p1; };

// Fixed decorations, remapped onto real heap offsets at pipeline-creation time via
// VkShaderDescriptorSetAndBindingMappingInfoEXT -- no set/binding number is meaningful to a human
// reading this file; it only needs to be consistent between this header and the C++ mapping builder.
layout(set = 0, binding = 0) uniform texture2D uTextureHeap[];
layout(set = 1, binding = 0) uniform sampler   uSamplerHeap[];
layout(set = 2, binding = 0) readonly buffer VtxView  { Vertex v[]; }             uHeap_Vertices[];
layout(set = 2, binding = 0) readonly buffer InstView { mat4 t[]; }               uHeap_Instances[];
layout(set = 2, binding = 0) readonly buffer MatView  { MaterialValues m[]; }     uHeap_Materials[];
layout(set = 2, binding = 0) readonly buffer CamView  { mat4 mat; vec4 camPos; }  uHeap_Camera[];
layout(set = 2, binding = 0) readonly buffer LgtView  { vec4 count; PointLight p[]; } uHeap_Lights[];
layout(set = 2, binding = 0) readonly buffer DrawView { DrawData d[]; }           uHeap_DrawData[];

const uint DEFAULT_SAMPLER = 0; // registered once at DescriptorHeap init time
vec4 SampleHeapTexture(uint texIdx, uint sampIdx, vec2 uv) {
    return texture(sampler2D(uTextureHeap[nonuniformEXT(texIdx)], uSamplerHeap[nonuniformEXT(sampIdx)]), uv);
}
```

(`pc.DrawID` from earlier drafts becomes whatever access pattern `vkCmdPushDataEXT`'s corresponding GLSL
side turns out to require — **this needs confirming once your SDK/driver support is in place**; it may
still look like a `push_constant`-style block from the shader's perspective, or it may need different
syntax. Don't guess this at implementation time — check the actual extension documentation/samples once
available.)

`pbr_heap.vert`/`.frag` each start with `#include "common/bindless_heap.glsl"` (already supported —
`ShaderCompiler.cpp:49-54` wires a glslang `DirStackFileIncluder` before parsing) and contain **no**
`layout(set=,binding=)` lines of their own. Logic otherwise mirrors `pbr.vert:25-39`/`pbr.frag:91-155`:
resolve the draw's `DrawData` first, then index `uHeap_Instances`/`uHeap_Vertices`/`uHeap_Camera` with
`nonuniformEXT(dd.*)`; in the fragment shader, look up `MaterialValues` via
`uHeap_Materials[nonuniformEXT(dd.Materials)].m[in_MaterialIndex]` and sample each texture via
`SampleHeapTexture(mat.AlbedoTex, DEFAULT_SAMPLER, in_TexCoord)`. Every dynamically-indexed heap access
must be wrapped in `nonuniformEXT(...)`.

On the C++ pass-declaration side (`Poly/src/Poly/RenderGraph/Pass/PassDeclaration.*`), a heap pass still
calls `WithShader(pbr_heap.vert/frag, ...)`, `WithGraphicsPipeline()`, and `MapResource(EFeaturePort::
Color/Depth, ...)` for its actual framebuffer attachments (real attachments, not heap resources, still
need attachment binding). `MapGlobal`/`ImportResource`/`ExportResource` remain only for
dependency/lifetime/sync tracking — they never emitted descriptor writes even before this change, so
nothing here changes on the C++ authoring surface.

## Decisions already confirmed with the user

- **Target extension:** `VK_EXT_descriptor_heap`, targeted directly (no interim backend on
  `VK_EXT_descriptor_buffer` or anything else) — you will update your Vulkan SDK yourself before
  implementation begins.
- **Heap count:** exactly one resource heap + one sampler heap (this is spec-mandated by the extension,
  not just a design preference) — all buffer shapes (vertices, instances, materials, camera, lights,
  draw data) share the one resource heap, accessed through multiple GLSL views aliasing the same
  `(set, binding)`.
- **No manual set/binding authoring:** all heap `layout(set=,binding=)` declarations live once in
  `assets/shaders/common/bindless_heap.glsl`, remapped onto real heap offsets via
  `VkDescriptorSetAndBindingMappingEXT`; individual shaders and pass declarations never specify
  set/binding themselves.
- **Synchronization:** the sync gap is closed as part of this work — `SampledImage`/`StorageBuffer` (and
  other heap-resident types) keep full barrier tracking in `PlanSynchronization`; only `Sampler`/
  `PushConstants`/`AccelerationStructure`/`SamplerFeedback` remain untracked.

## Needs your confirmation / verification once your SDK is updated

1. **Exact `VkPhysicalDeviceDescriptorHeapFeaturesEXT` field name(s)** and any additional required
   feature/extension dependencies (this plan's summary of the extension came from the public reference
   page, not the raw header — confirm the header directly once installed).
2. **Whether heap-mapped graphics pipelines still require a `VkPipelineLayout` handle** (even a trivial
   one) for `VkGraphicsPipelineCreateInfo::layout`, or whether the mapping-info structs supersede that
   requirement entirely.
3. **Exact shader-side syntax for `vkCmdPushDataEXT`'s corresponding GLSL access** (whether it still
   resembles a `push_constant` block or needs new syntax/extension string).
4. **Camera as a storage buffer** (functionally identical, different descriptor kind) — confirm
   acceptable, or say if `Camera`/other small per-frame-constant data should be handled differently.
5. **Heap capacities** — concrete slot counts for the resource heap and sampler heap.
6. **`DrawData` shape** — one `DrawID` → one entry with every index a draw needs (including per-frame
   camera/lights), vs. splitting into a per-frame `FrameData` + per-draw `DrawData` to avoid repeating
   camera/lights indices per batch.
7. **Streaming/hot-reload of heap slots** — whether `Release*`/free-list recycling is needed now (for
   future texture streaming) or can be deferred.
8. **Transient render-target textures entering the heap** — whether a graph-allocated transient texture
   that a later pass samples should auto-register into the resource heap on allocation.
9. **Legacy coexistence** — confirm a new heap-aware scene uploader is added alongside (not replacing)
   `RenderScene`, since the legacy graph/`RenderGraphProgram` keeps running side by side in
   `Renderer::Render()` today.

## Verification

No automated test suite exists in this repo — `Sandbox` is the manual test harness. Verify by:

1. Confirm `vkEnumerateDeviceExtensionProperties` lists `VK_EXT_descriptor_heap` for your GPU/driver
   before writing Vulkan-layer code (prerequisite check).
2. Build with validation layers enabled; confirm no `VUID` errors on device/heap creation.
3. Log/inspect `DescriptorHeap::RegisterTexture` calls during Sponza load — confirm every texture
   (including the default white one) gets a unique heap index, and each `Material`'s `MaterialValues.*Tex`
   fields match those indices.
4. Wire up a `pbr_heap` pass in `Sandbox/src/SandboxApp.cpp` against the new `RenderGraph`, load Sponza,
   and render it through the new `RenderProgramInstance`. Compare visually against the legacy `pbr.frag`
   output — correct albedo/normal/metallic/roughness/AO textures per material is the definitive proof
   that `DrawID → DrawData → MaterialValues.*Tex` resolves correctly through the shared resource heap.
5. Temporarily corrupt one material's `AlbedoTex` index and confirm exactly that material's texture
   changes — proves indexing is actually in effect, not incidentally-correct fixed binding.
6. Force a runtime graph recompile (new `RenderProgramInstance` swapped in) and confirm rendering is
   unaffected — validates that heap indices assigned at asset-load time survive instance replacement.
7. Move the camera / animate transforms for several seconds with validation layers on — confirm zero
   synchronization/hazard errors (validates the sync-tracking decision above).