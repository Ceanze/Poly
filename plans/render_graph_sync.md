# Render graph synchronisation

Explicit multi-queue synchronisation for the new render graph (`Poly/src/Poly/RenderGraph/`). Fills in the "Synchronisation" placeholder in [render_graph.md](render_graph.md).

## Context

`RenderProgramBuilder` (`Poly/src/Poly/RenderGraph/RenderProgramBuilder.cpp`) currently compiles a `RenderCatalog` + feature list into a flat, topologically-sorted `std::vector<ResolvedPass>` (`RenderProgram`), but nothing in that pipeline computes *how* resources should be synchronized between passes. `RenderProgramInstance::Execute()` is a complete stub — no GPU dispatch, resource allocation, or descriptor binding exists yet, so there is nothing downstream to wire barrier-issuing code into today. This plan is scoped to the compile-time half of the problem: extend `RenderProgramBuilder` with a fourth phase that computes a full **synchronization plan** — explicit barriers, queue-ownership transfers, and semaphore waits — for the already-sorted pass list, and attach it to `RenderProgram` so a future `Execute()` implementation can consume it. `RenderProgramInstance` is intentionally left untouched.

Motivation: the engine will use dynamic rendering, so there's no `VkRenderPass`/subpass-dependency mechanism to lean on for implicit layout transitions the way the legacy (dead) `Poly/src/Poly/Rendering/RenderGraph/` compiler does — everything must be explicit. The plan must also support real multi-queue (graphics/compute/transfer) execution, group independent barriers together where possible, and avoid redundant ("indirect") syncs when an earlier sync already established the required state/ordering.

Confirmed design decisions:
1. **Queue affinity** is declared per-pass via a new `IPassDeclaration::OnQueue(FQueueType)` builder method; defaults to `FQueueType::GRAPHICS` if unset.
2. **Cross-queue resources** use exclusive queue-family ownership transfer (`CommandBuffer::ReleaseTexture`/`AcquireTexture`), mirroring the existing pattern in `Poly/src/Poly/Resources/ResourceLoader.cpp`. No `VK_SHARING_MODE_CONCURRENT`.
3. **`RenderProgramInstance.cpp` is not modified.** All new logic lives in `RenderProgramBuilder`/`RenderProgram`; the output is pure data (`SyncPlan`).

## Reused building blocks (no new primitives invented)

- `FQueueType`, `FPipelineStage`, `FAccessFlag`, `ETextureLayout`, `FImageViewFlag`, `FShaderStage` — all in `Poly/src/Poly/Rendering/Core/API/GraphicsTypes.h`, already 1:1-mapped to Vulkan in `Platform/Vulkan/PVKTypes.h`.
- `CommandBuffer::PipelineBarrier` (batched, same-queue, multi-resource) and `AcquireTexture`/`ReleaseTexture`/`AcquireBuffer`/`ReleaseBuffer` (cross-queue ownership transfer, one call per resource, one call per queue side) in `Platform/API/CommandBuffer.h` — confirmed via the Vulkan spec + `PVKCommandBuffer.cpp` that a single `vkCmdPipelineBarrier` cannot itself perform a queue-family transfer; it's inherently a release-on-one-queue + acquire-on-the-other-queue pair, which is exactly why those methods exist separately.
- `FResourceState` (`Poly/src/Poly/RenderGraph/Resource/ResourceState.h`) — currently a dead, unwired D3D12-style enum set via `IResourceDeclaration::WithInitialState()`. This plan finally wires it up as a seed for a resource's tracked state on first use.

## New files

### `Poly/src/Poly/RenderGraph/Resource/ResourceUsage.h` / `.cpp`

```cpp
struct ResourceUsage
{
    ETextureLayout Layout     = ETextureLayout::UNDEFINED;
    FAccessFlag    Access     = FAccessFlag::NONE;
    FPipelineStage Stage      = FPipelineStage::NONE;
    FImageViewFlag AspectMask = FImageViewFlag::NONE; // texture-only, NONE for buffers
};

bool           IsTextureResourceType(EResourceType type);   // SampledImage/StorageImage -> true, else false
FPipelineStage DerivePassShaderStages(const std::vector<std::pair<std::string, FShaderStage>>& shaders);
ResourceUsage  DeriveResourceUsage(EResourceType type, bool isWrite, const std::string& resolvedName, FPipelineStage passShaderStages);
ResourceUsage  ConvertInitialState(FResourceState state);
```

`DeriveResourceUsage` special-cases `resolvedName == "$Color"` / `"$Depth"` / `"$Stencil"` (compare against `ToSemanticName(EFeaturePort::...)` from `Feature/FeaturePort.h`) *before* switching on `EResourceType`, since these ports are frequently externally-supplied and have `Type == EResourceType::None`:
- `$Color` → `COLOR_ATTACHMENT_OPTIMAL`, `COLOR_ATTACHMENT_READ|WRITE`, `COLOR_ATTACHMENT_OUTPUT`, aspect `COLOR`.
- `$Depth`/`$Stencil` → `DEPTH_STENCIL_ATTACHMENT_OPTIMAL`, `DEPTH_STENCIL_ATTACHMENT_READ|WRITE`, `EARLY_FRAGMENT_TEST|LATE_FRAGMENT_TEST`, aspect `DEPTH_STENCIL`.

Otherwise, switch on `EResourceType`:
- `SampledImage` → `SHADER_READ_ONLY_OPTIMAL` / `SHADER_READ` / `passShaderStages` / `COLOR`.
- `StorageImage` → `GENERAL` / `isWrite ? SHADER_WRITE : SHADER_READ` / `passShaderStages` / `COLOR`.
- `UniformBuffer`, `DynamicUniformBuffer` → no layout, `UNIFORM_READ`, `passShaderStages`.
- `StorageBuffer`, `StorageBufferReadWrite`, `RawBuffer`, `RawBufferReadWrite`, `UniformTexelBuffer`, `StorageTexelBuffer` → no layout, `isWrite ? SHADER_WRITE : SHADER_READ`, `passShaderStages`.
- `Sampler`, `PushConstants`, `AccelerationStructure`, `SamplerFeedback`, `None` → `Access = NONE` (no sync tracked; documented limitation — not used by any current pass, out of scope to solve here).

`DerivePassShaderStages` ORs `FShaderStage::VERTEX/FRAGMENT/COMPUTE` bits present in the pass's `Shaders` list into `FPipelineStage::VERTEX_SHADER/FRAGMENT_SHADER/COMPUTE_SHADER` (use the existing `BitsSet()` helper, same one used by `ConvertBindpointToDescriptorType` in `GraphicsTypes.h`); falls back to `FPipelineStage::ALL_COMMANDS` if nothing matched (defensive default, not expected to trigger for real passes).

`ConvertInitialState` maps the handful of `FResourceState` values that are meaningful for a texture/buffer barrier seed (`ShaderResource`, `UnorderedAccess`, `RenderTarget`, `DepthWrite`, `DepthRead`, `CopySource`, `CopyDest`, `Present`, `ConstantBuffer`) to a `ResourceUsage`; anything else falls back to a conservative `{GENERAL, MEMORY_READ|MEMORY_WRITE, ALL_COMMANDS, COLOR}` plus a `POLY_CORE_WARN`, guaranteeing a barrier is still inserted rather than assuming an incorrect layout.

### `Poly/src/Poly/RenderGraph/SyncPlan.h` / `.cpp`

```cpp
struct TextureTransitionPlan { std::string ResolvedName; ETextureLayout OldLayout, NewLayout; FAccessFlag SrcAccess, DstAccess; FPipelineStage SrcStage, DstStage; FImageViewFlag AspectMask; };
struct BufferTransitionPlan  { std::string ResolvedName; FAccessFlag SrcAccess, DstAccess; FPipelineStage SrcStage, DstStage; };

// One same-queue transition group == one future CommandBuffer::PipelineBarrier call.
struct BarrierGroup { std::vector<TextureTransitionPlan> Textures; std::vector<BufferTransitionPlan> Buffers; };

// One resource's half of a cross-queue ownership transfer (Release on src queue / Acquire on dst queue).
struct QueueReleasePlan { std::string ResolvedName; bool IsTexture; ETextureLayout OldLayout, NewLayout; FAccessFlag SrcAccess; FPipelineStage SrcStage; FQueueType DstQueue; };
struct QueueAcquirePlan { std::string ResolvedName; bool IsTexture; ETextureLayout OldLayout, NewLayout; FAccessFlag DstAccess; FPipelineStage DstStage; FQueueType SrcQueue; };

struct PassSyncPlan
{
    size_t                                    PassIndex = 0;
    BarrierGroup                              PreBarriers;    // same-queue transitions, batched into one call
    std::vector<QueueAcquirePlan>             Acquires;       // cross-queue acquires needed before this pass runs
    std::vector<QueueReleasePlan>             PostReleases;   // cross-queue releases to run right after this pass (attached here because this pass was the resource's last user, even though the consumer that needs the transfer is later)
    std::unordered_map<FQueueType, uint64_t>  RequiredWaits;  // per source-queue, min SyncPoint value to wait for before this pass runs; absent = no wait (this is where cross-queue "indirect sync" elision shows up)
    uint64_t                                  SubmissionIndex = 0; // 1-based position within this pass's own queue's submission order; doubles as the SyncPoint signal value once this pass's work is submitted
};

class SyncPlan
{
public:
    explicit SyncPlan(std::vector<PassSyncPlan> plans);
    const std::vector<PassSyncPlan>& GetPassPlans() const { return m_Plans; } // parallel to RenderProgram::GetPasses() - same size/order
private:
    std::vector<PassSyncPlan> m_Plans;
};
```

## Edits to existing files

**`Poly/src/Poly/RenderGraph/Pass/IPassDeclaration.h`** — add `virtual IPassDeclaration& OnQueue(FQueueType queue) = 0;` (doc comment: defaults to `FQueueType::GRAPHICS` if never called).

**`Poly/src/Poly/RenderGraph/Pass/PassDeclaration.h` / `.cpp`** — add `PassDeclaration& OnQueue(FQueueType queue) override;`, `FQueueType GetQueue() const { return m_Queue; }`, private `FQueueType m_Queue = FQueueType::GRAPHICS;`; `.cpp` body is a one-line `m_Queue = queue; return *this;` matching the file's existing style.

**`Poly/src/Poly/RenderGraph/RenderProgram.h` / `.cpp`**
- `#include "SyncPlan.h"`.
- Add `FQueueType Queue = FQueueType::GRAPHICS;` to `ResolvedPass`.
- Change constructor to `RenderProgram(std::vector<ResolvedPass> sortedPasses, SyncPlan syncPlan)`, add `const SyncPlan& GetSyncPlan() const { return m_SyncPlan; }` + `SyncPlan m_SyncPlan;`. Update `.cpp` to move-init both members.

**`Poly/src/Poly/RenderGraph/RenderProgramBuilder.h`** — add `SyncPlan PlanSynchronization(const std::vector<ResolvedPass>& passes) const;` as a fourth private phase, after `TopoSortALAP`.

**`Poly/src/Poly/RenderGraph/RenderProgramBuilder.cpp`**
- `#include "Resource/ResourceUsage.h"`.
- In `FlattenFeatures()`, add `resolved.Queue = pass->GetQueue();` alongside the existing `resolved.Name`/`Shaders`/`PipelineDesc`/`ExecuteFn` assignments.
- Implement `PlanSynchronization` — single forward walk over the sorted passes, tracking one `ResourceTrackState` per `ResolvedName` (names are already globally unique post-flattening, so no extra alias/canonicalization layer is needed, unlike the legacy compiler's `ResourceCache::GetCanonicalGUID`):

```cpp
struct ResourceTrackState
{
    bool           HasState      = false;
    bool           IsTexture     = false;
    ETextureLayout Layout        = ETextureLayout::UNDEFINED;
    FAccessFlag    Access        = FAccessFlag::NONE;
    FPipelineStage Stage         = FPipelineStage::NONE;
    FQueueType     Queue         = FQueueType::GRAPHICS;
    bool           LastWasWrite  = false;
    size_t         LastPassIndex = 0;
};

SyncPlan RenderProgramBuilder::PlanSynchronization(const std::vector<ResolvedPass>& passes) const
{
    std::unordered_map<std::string, ResourceTrackState> state;
    std::unordered_map<FQueueType, uint64_t>            queueSubmitCounter;
    std::unordered_map<FQueueType, std::unordered_map<FQueueType, uint64_t>> highestWaited;
    std::vector<PassSyncPlan> passPlans(passes.size());

    for (size_t i = 0; i < passes.size(); ++i)
    {
        const ResolvedPass& pass  = passes[i];
        const FQueueType    queue = pass.Queue;
        passPlans[i].PassIndex       = i;
        passPlans[i].SubmissionIndex = ++queueSubmitCounter[queue];

        const FPipelineStage passStages = DerivePassShaderStages(pass.Shaders);
        std::unordered_map<FQueueType, uint64_t> neededWaits;
        std::unordered_set<std::string> seenThisPass;

        for (const ResolvedPort& port : pass.Ports)
        {
            if (!seenThisPass.insert(port.ResolvedName).second)
            {
                POLY_CORE_ERROR("Pass '{}' reads and writes resource '{}' within the same pass; "
                                "intra-pass synchronization isn't supported, skipping.", pass.Name, port.ResolvedName);
                continue;
            }

            const bool isAttachment = /* resolvedName == "$Color"/"$Depth"/"$Stencil" */;
            const bool isTexture    = isAttachment || IsTextureResourceType(port.Type);
            if (!isAttachment && port.Type == EResourceType::None)
                POLY_CORE_WARN("Pass '{}' port '{}' has no resource type; it will not be synchronized.", pass.Name, port.ResolvedName);

            ResourceUsage needed = DeriveResourceUsage(port.Type, port.IsWrite, port.ResolvedName, passStages);
            ResourceTrackState& rs = state[port.ResolvedName];
            if (!rs.HasState)
            {
                rs.HasState  = true;
                rs.IsTexture = isTexture;
                rs.Queue     = queue; // first touch: assume no incoming cross-queue dependency
                if (port.InitialState != FResourceState::Unknown)
                {
                    ResourceUsage seed = ConvertInitialState(port.InitialState);
                    rs.Layout = seed.Layout; rs.Access = seed.Access; rs.Stage = seed.Stage;
                }
            }

            const bool queueDiffers  = rs.Queue != queue;
            const bool layoutDiffers = isTexture && rs.Layout != needed.Layout;
            const bool isHazard      = rs.LastWasWrite || port.IsWrite || layoutDiffers;

            if (queueDiffers)
            {
                passPlans[rs.LastPassIndex].PostReleases.push_back({port.ResolvedName, isTexture, rs.Layout, needed.Layout, rs.Access, rs.Stage, queue});
                passPlans[i].Acquires.push_back({port.ResolvedName, isTexture, rs.Layout, needed.Layout, needed.Access, needed.Stage, rs.Queue});
                uint64_t& wait = neededWaits[rs.Queue];
                wait = std::max(wait, passPlans[rs.LastPassIndex].SubmissionIndex);
            }
            else if (isHazard)
            {
                if (isTexture) passPlans[i].PreBarriers.Textures.push_back({port.ResolvedName, rs.Layout, needed.Layout, rs.Access, needed.Access, rs.Stage, needed.Stage, needed.AspectMask});
                else            passPlans[i].PreBarriers.Buffers.push_back({port.ResolvedName, rs.Access, needed.Access, rs.Stage, needed.Stage});
            }
            // else: resource is already exactly where it needs to be -- indirect sync, nothing to do.

            rs.Layout = needed.Layout; rs.Access = needed.Access; rs.Stage = needed.Stage;
            rs.Queue = queue; rs.LastWasWrite = port.IsWrite; rs.LastPassIndex = i;
        }

        for (const auto& [srcQueue, neededValue] : neededWaits)
        {
            uint64_t& already = highestWaited[queue][srcQueue];
            if (neededValue > already) { passPlans[i].RequiredWaits[srcQueue] = neededValue; already = neededValue; }
            // else: this queue already waited far enough -- cross-queue indirect sync elision.
        }
    }

    return SyncPlan(std::move(passPlans));
}
```

- Wire into `Build()`:
```cpp
std::unique_ptr<RenderProgram> RenderProgramBuilder::Build()
{
    auto flat     = FlattenFeatures();
    auto nodes    = BuildDAG(flat);
    auto sorted   = TopoSortALAP(nodes, flat);
    auto syncPlan = PlanSynchronization(sorted);
    return std::make_unique<RenderProgram>(std::move(sorted), std::move(syncPlan));
}
```

## How this satisfies the spec

1. **Explicit-only sync**: every transition is produced as barrier/acquire/release data, never as render-pass attachment load/store metadata.
2. **Resource sync between passes**: driven by a per-resource state machine (`ResourceTrackState`) keyed by the already-canonical `ResolvedName`.
3. **Multi-queue**: `queueDiffers` branch emits a `QueueReleasePlan` (on the resource's previous owning pass) + `QueueAcquirePlan` (on the consuming pass) + a `SyncPoint` wait requirement, mirroring the existing `ResourceLoader.cpp` release/acquire/semaphore pattern.
4. **Grouped syncs**: same-queue transitions for a pass are batched into one `BarrierGroup` (one future `PipelineBarrier` call covering N resources — e.g. pass1 writes A, pass2 writes B, pass3 consumes both → one `BarrierGroup` before pass3 with two entries). Cross-queue: multiple resources crossing from the same source queue collapse into one `RequiredWaits` entry (one wait, not N), even though each resource still needs its own Acquire/Release call pair (Vulkan has no batched form for ownership transfer).
5. **Indirect syncs**: same-queue — the `isHazard` test (`priorWrite || neededWrite || layoutChange`) skips the barrier entirely when a resource is already in the right state (e.g. pass3 reading `A` after pass2 only read it — no transition needed since pass1→pass2's barrier already established the state pass3 needs). Cross-queue — the `highestWaited[queue][srcQueue]` map skips a wait if an earlier pass on the same consuming queue already waited far enough.

## Known, explicitly out-of-scope limitations (documented, not solved here)

- **Same-pass read+write of one resolved name** (a pass that both imports and exports the same name): detected and rejected with `POLY_CORE_ERROR` rather than silently mis-scheduled, since it needs a genuine intra-pass barrier point that the current single-phase-per-pass model doesn't have.
- **A resource consumed by 3+ different queues after one write**: exclusive-family ownership can only be held by one queue at a time, so the algorithm naturally serializes it into a release→acquire→release→acquire chain in pass order. This is an inherent cost of exclusive sharing (the alternative, `CONCURRENT`, was explicitly ruled out) — not a bug.
- **`Sampler`/`PushConstants`/`AccelerationStructure`/`SamplerFeedback`** ports get no barrier tracking (no current pass uses them this way).

## Verification

No automated test suite exists in this repo; this is pure build-time data with no GPU dispatch yet, so verification is:
1. **Compiles** — `RenderProgram`'s constructor signature changes; confirm `RenderProgramBuilder.cpp` is the only call site (grep for `RenderProgram(` / `make_unique<RenderProgram>`).
2. **Hand-trace the spec's worked examples** against `PlanSynchronization`:
   - *Grouped*: pass1 writes A, pass2 writes B (independent), pass3 reads A+B, all same queue → `passPlans[2].PreBarriers` has two entries; `passPlans[0]`/`passPlans[1]` empty (first touch).
   - *Indirect*: pass1 writes A, pass2 reads A + writes B, pass3 reads A+B → no transition for A into pass3 (state unchanged since pass1→pass2's barrier), only B gets one.
   - *Cross-queue*: pass1 (TRANSFER) writes A, pass2 (GRAPHICS) reads A → `passPlans[0].PostReleases` gets one entry, `passPlans[1].Acquires` gets one entry, `passPlans[1].RequiredWaits[TRANSFER] == passPlans[0].SubmissionIndex`.
3. Temporarily add a `POLY_CORE_TRACE` dump over `program->GetSyncPlan().GetPassPlans()` in `SandboxApp.cpp` right after building the existing feature graph, to eyeball real output — remove it once confirmed (not permanent scaffolding). Confirm no unexpected `POLY_CORE_ERROR`/`WARN` fires for the current Sandbox graph (no pass calls `OnQueue` yet, so everything lands on `GRAPHICS`; no pass currently imports+exports the same name).
