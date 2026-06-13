# Render graph
New render graph architecture, covering APIs, functionalites, examples, etc.

## Problem
The current render graph has several problems which means that it doesn't fit well for a modern game engine. Some of the problems are;

- Hard coded - the graph requires new classes in code to add new functionality
- Static - the graph requires compilation both in graph but also in code to make changes. This makes disabling a pass current impossible without a full redefinition of what the pass is doing.
- Not game ready - the current architecture is only designed for render engineers, to just have a simple scene a complex render graph needs to be set up
- Does not handle multiqueue or multithreading
- Uses old tech - no bindless, proper sync, etc.
- Editing a RG - removing a pass in the middle requires changing all connections manually, creating a lot of extra work for disabling something.
- To implicit - a lot of things are done implicitly, which means as soon as something needs to be done explicitly, like in the ImGui pass, a lot of extra cases need to be considered.

## Requirements

- **Data driven** - a render graph should be able to be defined using data instead of requiring a hard recompile
- **Bindless** - Use the new bindless option in Vulkan to enable a simpler Descriptor approach per pass
- **Multithreading** - Each pass recording should be done on a separate thread, using sublists, which are then combined to a main command list/buffer before submission.
- **Multiqueue** - Each pass type should match the possible queues. Compute, Render, and Transfer are all required to work, and all of these should execute on the correct queue type. Important here is getting syncronisation and async working across queues. According to [Loggini](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html) the automatisation of queue transfer is surprisingly complex, and manual definition for each pass is preferred.
  - > We could make the graph automatically handle async compute operations but turns out this is not optimal and manual settings are preferred.
- **Transient resources with aliasing** - transient (resources created by the render graph) should be aliased, meaning it can be used for multiple purposes. For instance if Pass 1 writes Texture A, Pass 2 consumes it, and Pass 3 writes another texture, then, since Texture A is already consumed, it can reuse the same memory.
- **Dynamic recreation** - the render graph should only be recreated/recompiled when needed. Common approach is to define the RG every frame, but only recompile it if the computed hash (pre optimization) is different.
- **Explicit and simple mode** - Explicit underlying system that clearly specifies what each pass does per pass level, together with a simple mode wrapper that supports a simpler builder like `AddBloom()` or `AddPass(bloom)`
- **Render Feature** - a render feature is essentially a sub-graph. It defines a set of inputs, outputs, and internally used render passes. For a user a render feature, like "Bloom" can be added without having to know the specific passes that are used.
  - Features are simple by default - this means `AddFeature(feature)` is all that is required. The feature might consume/produce several resources, but these should be common and already existing, for instance `SceneColor` using semantic naming. Each resource can then be further changed if needed to set it up, so `AddFeature(feature).Source(offscreen)` is valid (source might need more info, but point stands).
- **Resource definition with strings** - there are two main ways to definte resource usage by passes/features. First is to use actual handles to the concrete defined resources. This allows for a solid safety, but lacks flexibility. Instead the second option, defining and using resources using strings, allows for independent modules that can be added and removed without having to handle other passes. This can further be expanded by allowing overrides from outside the pass, so that if for instance the defined "ColorPass" in the pass definition doesn't exist, it can be mapped outside by something like `(pass).Source("ColorPass", res)`
- **Render View/Context for execute** - current render graph in Poly assumes always one scene and one camera, rendering multiple of either breaks the graph. Solve this by having a RenderView/Context that is provided for each execute. The view provides renderCamera (created from a camera), renderScene, renderTarget (swapchain if not specified), viewport, clear options,

## Further research
- Bindless, how it works, how it implements, and how the pattern can be applied for a render graph structure with individual render passes.
- Multi command buffer recording with multithreading

## New API

### Simple case
Covers a simple PBR + ImGui (current SandboxApp) setup from a user perspective and from an underlying perspective

```c++
// Backend setup
auto passRegistry = PassRegistry::Create();
passRegistry.RegisterPass("pbr", setupFn, executeFn);
passRegistry.RegisterPass("imgui", setupFn, executeFn);

// User facing
auto renderGraph = RenderGraph::Create();

// Begins "recording" a potentially new graph
auto renderProgram = renderGraph.Begin()
  .AddPass("pbr") // uses globally accessable resources like sceneAo, sceneVert, camera, etc.
  .AddPass("imgui")
  .Build();

Renderer::SetRenderProgram(renderProgram);
Renderer::Render();

// --------- Renderer::Render()
// Execute program with context. Context includes window, frameIdx, renderScene, renderCamera, etc.
renderProgram.Execute(renderContext);

```

### Pass creation
```c++
auto passRegistry = ...
passRegistry.RegisterPass(
  "pbr",
  [](SetupContext& ctx){
    // Sets shader, reflects it, and uses reflection for available input resources, with optional aliasing
    ctx.SetVertexShader(vertShader)
      .AliasResource("albedoTex", "sceneAlbedo")
      .AliasResource("vertices", "sceneVertices");

    ctx.SetFragmentShader(fragShader)
      .AliasResource("out_Color", "color");

    // Gets or defines a new resource and specify properties, overriding defaults
    ctx.Resource("out_Color").AsTexture()
      .Format(EFormat::B8R8G8A8_UNORM)
      .Usage(ETextureUsage::COLOR_ATTACHMENT);

    ctx.Resource("depth").AsTexture()
      .Format(EFormat::DEPTH_STENCIL)
      .Usage(ETextureUsage::DEPTH_STENCIL_ATTACHMENT);
  },
  [](const RenderContext& ctx){
    // Execute is called once per frame only, any additional logic that requires looping is done in the callback
    for (auto& batch : ctx.RenderScene->GetBatches()) { ... } // Bind, draw
  }
);

// TODO: ImGui example with custom pipeline (just vertex inputs?) and external resources such as Font
```

### Data driven
#### Problem
Ideally, everything should be data driven and passes should be able to be created using just data. However, there is one limitation that makes a feature-complete data driven system less ideal - namely the Execute function.

The execute function is a pure c++ function, that can do any type of logic, and uses the context only for the inner most call. Compare this to the Setup function, where essentially everything is done using the context.

Take the PBR pass as an example, it currently uses the SceneRenderer, which (although indirectly, but it is still instanced) goes through all instances/batches and calls draw on them, binding specific data. This logic uses pointers, offsets, specific memory, for loops, etc. This cannot be seralized in a meaningful way without essentially using another language to handle the logic, like a script.

#### Solution
The solution is relatively simple - make every that can be data driven, data driven. For the pass creation this means everything except the content of the execute function. The name, type, setup (input, output, pipeline overrides, etc), and importantly, the execute function handle. The Execute function handle is a simple ID/name to a predefined executor, for instance for the PBR pass this would be `SceneRaster`, for the ImGui it would be a specific like `ImGui`.

Most executors should be generic, so they can be reused, for instance a `FullscreenQuad` executor would be used for all post processing effects.

On the code side there will be two places (engine and user) to define new executors, and definition is done potentially like:

1. Create function callback, using just globally accessable data and the provided context as data.
2. Register callback to the system in code; `ExecutorRegistry.AddExecutor("SceneRaster", fn);`.
3. Add registered callback name and description to a database file like `executors.yaml`, specifying `name: "SceneRaster" description: "Raster the scene"`

Then the pass file can reference this `SceneRaster` in the executor part.

#### Full pass example
```yaml
pass:
  name: pbr
  type: Render
  vertexShader: pbr.vert
  fragmentShader: pbr.frag
  inputs: # Gotten from shader, list defines alias, formats, etc, if needed
    albedoTex:
      alias: sceneAlbedo
    vertices:
      alias: sceneVertices
    position: # Position and UV are vertex inputs; this is not part of MVP
      type: VERTEX_INPUT
      location: 0
    uv:
      type: VERTEX_INPUT
      location: 1
      offset: 24
  outputs: # Also gotten from shader, but shader does not know format nor specific usage, always assumes color attachment
    out_Color:
      alias: color
      format: B8R8G8A8_UNORM
      usage: COLOR_ATTACHMENT
    depth:
      format: DEPTH_STENCIL
      usage: DEPTH_STENCIL_ATTACHMENT
  pipeline: # Overrides. PBR does not have any, these are just examples
    rasterPolygonMode: FILL
    colorBlendAttachments:
      - blendEnable: false
      - blendEnable: true
        colorBlendOp: ADD
```

### Registries
To support both the data driven and code ways to handle passes, features, and resources, several (or one big) registries are needed.

- Pass registry
  - Registers fully definied passes `passRegistry.RegisterPass("pbr", setupFn, executeFn);`
  - Needed to support user facing code like `AddPass("pbr");`
- Executor registry
  - Registers executors that can be used for passes `executorRegistry.RegisterExecutor("SceneRaster", executeFn);`
  - Needed to support data driven model and reusable executors
  - Pass registering would then allow a direct function, or an ID to an already registered executor
- Resource registry
  - Registers global (external/imported) resources to be used by passes `resourceRegistry.RegisterResource("sceneAlbedo")`
  - Actual resource is NOT added at registration, it is added later with a direct call to the render program `renderProgram.UpdateResource("sceneAlbedo", renderScene.SceneAlbedoRes)`. At render graph creation only name and initial/final use is of interest.
  - The user (or program) needs to have the resources ready by the first execute call

### Features
A "feature" is a group of passes that act as one big pass. The purpose of this functionality is to abstract away the concept of graphics API details to the user as much as possible. A "feature" has no specifics about rendering, graphics api, resources, etc. An example of a feature would be "Blur", "Bloom", "Shadows", "ScenePBR", etc.

```c++
auto geometry = featureRegistry.New("geometry")
  .AddPass("pbr")
  .Build();

auto bloom = featureRegistry.New("bloom")
  .AddPass("BlurH")
  .AddPass("BlurV")
  .AddPass("Composite")
  .Build();
```

```c++
renderGraph.Begin()
  .AddFeature("geometry")
  .AddFeature("bloom")
  .Build();
```

#### Feature & pass mapping
To make a truly user-facing "drop in feature" functionality possible, the resources need to connected themselves. From a pass perspective this isn't doable, since that would require shaders and outputs from shaders to be written with the rest of the program in mind, instead of isolated units.

To handle this the feature layer will do the mapping.

When defining a feature, the passes are added, and any mapping that needs to happen is done.

For **internal connections** passes names are connected between each other, similar to how the current Render Graph works. For **connections that map outside**, like "Color", "Depth", etc. there are semantic mappsing available, that are consistent across features. These are a small group of always mappable resources that features can use between them to implicitly connect. For **global** connections the feature state which global resource it reads/writes, and then handles the mapping the same way as the internal.

Example
```c++
Feature::New("geometry")
  .AddPass("pbr")
    .Map(FeaturePort::Color, "out_Color")
    .Map(FeaturePort::Depth, "depth")
    .MapGlobal("SceneAlbedo", "albedoTex")
    .MapGlobal("SceneAO", "aoTex)
```

## Compilation/Setup
Just like before, the graph needs to be compiled in order to setup the neccessary resources, transitions, barriers, ordering, and culling. Here the algorithm for this will be outlined in psuedo-code like fasion.

1. Given an ordered container of passes is provided, and global resources are set up
2. Setup pass connections from implicit -> explicit, creating a graph. All writes are also optional inputs
3. For each pass in reverse ordered container
   1. Add to the set if mandatory or writing to swapchain
   2. If pass is already added to visited set, add dependent passes to the set

// Cull and setup the edges at the same time - go through all mandatory passes, check read/writes, add those passes to the stack, continue. This only adds the passes that contribute to the mandatory ones

### Prequesites
```c++
// Passes
passRegistry.RegisterPass("pbr",
  [](SetupContext& ctx) {
    ctx.ReflectShader("pbr.vert"); // reads albedoTex, vertices inputs
    ctx.ReflectShader("pbr.frag"); // writes out_Color, implicit depth
  },
  [](ExecuteContext& ctx) { ... }
)

passRegistry.RegisterPass("imgui",
  [](SetupContext& ctx) {
    ctx.ReflectShader("imgui.vert"); // reads debugData input
    ctx.ReflectShader("imgui.frag"); // writes fColor output
  },
  [](ExecuteContext& ctx) { ... }
)

// Features (also registers elsewhere to registry)
Feature::New("geometry")
  .AddPass("pbr")
    .Map(FeaturePort::Color, "out_Color")
    .Map(FeaturePort::Depth, "depth")
    .MapGlobal("SceneAlbedo", "albedoTex")
    .MapGlobal("SceneVertices", "vertices");

Feature::New("debug")
  .AddPass("imgui")
    .Map(FeaturePort::Color, "fColor")
    .MapGlobal("DebugData", "debugData");

// Register globals
resourceRegistry.RegisterResource("DebugData")
resourceRegistry.RegisterResource("SceneAlbedo")
resourceRegistry.RegisterResource("SceneVertices")

// Render Graph
auto renderProgram = renderGraph.Begin()
  .Feature("geometry")
  .Feature("debug")
  .Build();

// Execute
renderProgram.UpdateResource("SceneAlbedo",   renderScene.GetAlbedoTexture());
renderProgram.UpdateResource("SceneVertices", renderScene.GetVertexBuffer());
renderProgram.UpdateResource("DebugData",     imguiData);

RenderContext ctx { .Camera = camera, .Scene = renderScene, .Target = swapchain };
renderProgram.Execute(ctx);
```

### 1. Flatten graph
Go through all the features, explode them, add to a vector, continue.

Must be recursive for the explosion to support feature-in-features.

The explosion must handle the connection names properly. Given the prequesites, we get this

| Pass  | Port            | Resolved name |
| ----- | --------------- | ------------- |
| pbr   | albedoTex (in)  | SceneAlbedo   |
| pbr   | vertices (in)   | SceneVertices |
| pbr   | out_Color (out) | $Color        |
| pbr   | depth (out)     | $Depth        |
| imgui | debugData (in)  | DebugData     |
| imgui | fColor (out)    | $Color        |

internal feature passes mapping, for instance if Geometry feature has two passes, pbr1 and pbr2, and they have a connection those are defined like so:

| Pass | Port      | Resolved name | type                                            |
| ---- | --------- | ------------- | ----------------------------------------------- |
| pbr1 | res (out) | geometry.res  | implicit                                        |
| pbr2 | res (in)  | geometry.res  | implicit                                        |
| pbr1 | out (out) | geometry.in   | explicit mapping (mapped out -> in, explicitly) |
| pbr2 | in (in)   | geometry.in   | implicit                                        |

### 1. Create graph

Given pass outputs (Color) are in AvailableInputs.

Given features are flattened - i.e. are only passes with correctly defined and mapped ports (no features)

The graph creation successfully culles the unnecessary passes at the same time

- For each pass in reverse order
  - If current pass is mandatory or writes to a resource in AvailableInputs
    - For each write
      - If write has matching resources in AvailableInputs
        - Link the resources
        - Remove the resources in AvailableInputs
    - For each port
      - Add to AvailableInputs

### 2. Ordering

Step 1 provides a technical correct order of passes, but not the most optimal. Given the passes ABCD, where A -> D, B -> C -> D, it is more optimal to execute the passes in BCAD order. This is because we can alias resources easier the shorter timespan they have. Executing A close to D means resource lifetime is just between A and D, instead of A, B, C, and D.

To do this, schedule as late as possible, using a computed depth when doing topological sort.

```rust (not really)
function ComputeDepth(pass) -> int:
  if pass has no dependencies: return 0
  return 1 + max(ComputeDepth(dep) for dep in pass.dependencies)

// Topological sort, but when choosing next pass,
// always pick the one with the highest depth first
function TopoSortALAP(passes) -> Pass[]:
  result = []
  visited = {}

  function Visit(pass):
    if pass in visited: return
    visited.add(pass)
    // Sort deps by depth descending — deepest chain first
    deps = sorted(pass.dependencies, key=ComputeDepth, descending=true)
    for dep in deps:
      Visit(dep)
    result.append(pass)

  sinks = passes where nothing depends on them
  for sink in sinks:
    Visit(sink)
  return result
```

### Synchronisation



## Sources
Render Graph: https://logins.github.io/graphics/2021/05/31/RenderGraphs.html

Memory aliasing: https://levelup.gitconnected.com/gpu-memory-aliasing-45933681a15e

Render graph DAGs explained in detail: https://levelup.gitconnected.com/organizing-gpu-work-with-directed-acyclic-graphs-f3fd5f2c2af3

## Q&A
- Check how resources are connected between passes. We want to go away from the strict "pass B uses pass A resource" to a more generic "pass A produces resource X. pass B consumes resource X", i.e. the resource origin does not matter for the pass.
  - Claude answer; Passes use global namespaces when used, but when they are inside a feature, they are scoped to that feature. This makes normal drop and replace work well, but removes any collision between other features.

```c++
auto bloom = featureRegistry.New("bloom")
  .AddPass("blur_h")
  .AddPass("blur_v")
  .AddPass("composite")
  .Export("composite.color", "bloomOutput")  // global name: "bloomOutput"
  .Build();
```

- Understand how a passthrough is defined in the setup phase. Calling `read()` and `write()` does technically create a passthrough, but it also doesn't fully reflect what the pass is doing. It cannot read and write (assuming normal texture) to the resource, but rather just write. We do not however want to define that a resource is a passthrough, since it might not always be.
  - Claude answer; the concept of passthrough from a reflection/pass perspective is no longer needed. Instead the order of the pass definition and simple `Write()` calls is enough.
    - For instance, if PBRPass has a `Write("color")`, and ImGuiPass, positioned after, has the same, then that resource is a passthrough, unless ImGui does `Write("color", EClearOp::CLEAR)`.
    - If PBRPass is removed and we just have the ImGui pass, then the op in ImGuiPass becomes a simple output, no passthrough, automatically. If the PBRPass would be placed after the ImGui pass, it would instead become passthrough. It is order dependent.


## NVRHI migration
Swapchain

Keep your custom SwapChain/PVKSwapChain entirely. NVRHI explicitly does not own the swapchain — that's by design. What changes is the boundary:

- After acquiring a backbuffer image, wrap the VkImage as an NVRHI texture using nvrhi::vulkan::createTextureFromNativeObject() so the rest of the pipeline can treat it as a normal nvrhi::TextureHandle.
- Present still goes through your VkQueue/VkSemaphore path. NVRHI's ICommandList::close() + device flush happens before your present call.
- Resize: recreate the swapchain as now, then re-wrap the new VkImages as NVRHI textures.

---
Window

No changes needed. NVRHI is completely windogives the VkSurfaceKHR to the swapchain, andthat's the only coupling.

---
DescriptorCache

Eliminate it entirely. This is the biggest conceptual shift. NVRHI's model:

- You declare a BindingLayoutHandle per pass (what slots exist, what types).
- You create a BindingSetHandle per draw (which concrete resources fill those slots).
- For bindless (which your new RG targets): use nvrhi::DescriptorTableHandle — a GPU-visible heap of descriptors you can index into shaders freely.

Your current cache solved Vulkan descriptor pool lifetime/reuse. NVRHI's Vulkan backend manages the pool internally. The per-frame sync tracking (SyncPointValue, BUFFER_LIFETIME) goes away — NVRHI's IDevice defers destruction until the GPU finishes via its own refcount + fence tracking.

Migration path: replace DescriptorCache::Getset, ...) call sites withdevice->createBindingSet(desc, layout). The new RG's pass SetupContext should produce BindingLayoutHandles during setup and the ExecuteContext/ICommandList uses them at draw time.

---
StagingBufferCache

Eliminate it. NVRHI absorbs this responsibility:

- ICommandList::writeBuffer(dst, data, size) — NVRHI internally manages a staging pool per command list.
- ICommandList::writeTexture(dst, ...) — same for textures.
- For the ResourceLoader texture upload path: create the texture with device->createTexture(), open a command list, call writeTexture, close and execute — no manual staging buffer needed.

The per-swapchain-image tracking (imageIndexy because NVRHI tracks which command listsare in-flight via its own fence/queue abstraction.

---
FramebufferCache

Simplify, don't eliminate. NVRHI has IFramebuffer created via device->createFramebuffer(FramebufferDesc), but it doesn't cache them internally. Your hash-based cache is still useful to avoid recreating them every frame.

Simplification: the GraphicsRenderPass* key ffer doesn't need a pre-declared render passobject (that's a D3D11/Vulkan render pass concern NVRHI abstracts away). The new key is just (attachments[], depthAttachment). The hash can be over nvrhi::TextureHandle pointers directly.

In the new render graph, the compiler createtion phase (not per-frame), so the cache mayshrink to a compile-time map rather than a runtime LRU.

---
ResourceLoader
 file I/O layer (stb_image, Assimp, YAML, GLSLang) stays unchanged. What you replace:

- Texture creation: device->createTexture(nvrhi::TextureDesc{...}) instead of GraphicsInstance::CreateTexture()
- GPU upload: cmdList->writeTexture() instead of StagingBufferCache::QueueTransfer()
- Buffer creation: device->createBuffer(nvrhi::BufferDesc{...})
- The transfer command pool/semaphore sync: replaced by NVRHI command list execute + device->waitForIdle() or a fence wait

Shader loading stays as SPIR-V bytes — NVRHI accepts nvrhi::ShaderDesc + SPIR-V blob directly via device->createShader().

---
New Render Graph implications

The new RG design aligns well with NVRHI. Concrete touchpoints:

Pass setup → produces nvrhi::BindingLayoutHandle per pass (from shader reflection via SPIRV-Reflect, which you already
have). The SetupContext should store these.

Pass execute → receives an nvrhi::CommandListHandle. The execute function calls cmdList->setGraphicsState(...), cmdList->drawIndexed(...) etc. The GraphicsState struct takes your pipeline, framebuffer, and binding sets inline — no separate bind calls.

Transient aliasing → NVRHI has nvrhi::IHeap (placed resources) for D3D12; on Vulkan you'd use VK_KHR_bind_memory2 with aliased allocations. NVRHI doesn't abstract this directly today, so aliasing will require a thin Vulkan-side layer on top of NVRHI, or you alias at VMA level before wrapping as NVRHI textures via createTextureFromNativeObject.

Multiqueue → NVRHI command lists are queue-typed: nvrhi::CommandQueue::Graphics, ::Compute, ::Copy. The RG pass type maps directly to one of these. Cross-queue sync is device->waitForIdle() at coarse granularity, or signal/wait via
nvrhi::EventQueryHandle for fine-grained.

Bindless → device->createDescriptorTable(layout) + device->writeDescriptorTable(table, items). The resource registry's global resources get indices into this table. Passes read by index (push constant or UBO with the index). This
replaces per-pass descriptor set allocation

Resource states → NVRHI tracks TextureState/BufferState per-subresource internally and inserts Vulkan barriers when you transition between uses on a command list. The RG compiler's sync injection step becomes: call cmdList->setTextureState(tex, subresources, newState) before each pass, and NVRHI handles the actual vkCmdPipelineBarrier. This substantially simplifies the compiler's synchronization phase.

---
Recommended order of attack

1. Init NVRHI Vulkan device wrapping your exthis can coexist with your PVK layer duringtransition.
2. Migrate ResourceLoader to produce nvrhi::ndle — isolated change, validates the deviceintegration.
3. Drop StagingBufferCache, use cmdList->writeTexture/Buffer.
4. Migrate FramebufferCache to nvrhi::IFramebuffer.
5. Build the new RG's pass execution on nvrhi::ICommandList — the new RG is greenfield so you're not migrating, just building natively on NVRHI from the start.
6. Drop DescriptorCache once the new RG is t
7. Swapchain and Window stay indefinitely.