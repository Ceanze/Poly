# Multi-view support for RG2

Support for multiple views is required for normal render flows, and especially for the editor support.

The main issue with views is that they aren't defined until runtime when being submitted to the renderer, while a render graph is normally defined and compiled before this into a constant object.

To support dynamic views the render graph needs to be compiled at a later stage, while also being finalised at a later stage. This means that a full definition of every single pass isn't supplied by the user in one step, but rather some sort of metadata instead.

## Scope

In scope:

- Several views per frame, each with its own camera, world and target.
- Views rendering to the window or to an offscreen texture (editor viewport, portal).
- One view reading the output of another.
- Cheap switching between view combinations that have been seen before.

Out of scope for now:

- Multiple windows. The plan assumes a single window; the per-window `WindowContext` stays as it is, but only one window gets a render plan.
- Renderer-internal views (shadow maps, cubemap faces, cascades). They should fit the same model later, see open questions.
- Per-view culling. Draw batches stay per world.

## Decisions

| Topic            | Decision                                                                                             |
| ---------------- | ---------------------------------------------------------------------------------------------------- |
| Frame definition | The whole frame is planned up front in a `RenderPlan`: frame features and view stages, in order       |
| What a view renders | Decided by the stage it belongs to. There are no templates and no per-submit feature choice       |
| Compile unit     | All submitted views of a frame are flattened into one merged `RenderProgram`, cached by structure    |
| Targets          | Optional per view. Unset renders to the window's shared `$Color` with a viewport rect, set renders to an offscreen texture |
| View identity    | Persistent `RenderView` object, created by the renderer and held by the caller                       |
| Dependencies     | Plan order first. Within a stage, a view reading another view's target is ordered after it, submit order as tie break |
| Name scoping     | Declared on the resource: `Global`, `World` or `View`                                                |
| Switch cost      | Pipelines and view transients are hoisted out of `RenderProgramInstance`, so a switch is cheap       |

## Design

### The render plan

The render plan is the metadata the user supplies instead of a finished render graph. It lists, in execution order, everything a frame can contain:

- **Frame features** run exactly once per frame, whether or not any view is submitted. They have no view and no world, and render to the window.
- **View stages** run once per view submitted to them. A stage is a named list of features. A stage no view was submitted to contributes no passes.

```c++
Ref<RenderPlan> pPlan = m_Graph.BeginRenderPlan()
                           .AddFrameFeature(FrameSetup::FEATURE_NAME)
                           .AddViewStage("Scene")
                               .AddFeature(GeometryFeature::FEATURE_NAME)
                               .AddFeature(PostProcessFeature::FEATURE_NAME)
                           .EndStage()
                           .AddFrameFeature(ImGuiLayer::FEATURE_NAME)
                           .Build();

pRenderer->SetRenderPlan(pPlan);
```

`BeginRenderPlan()` lives on `RenderGraph`, next to today's `Begin()`, since the plan needs the catalog to look features up. `AddViewStage()` returns a `ViewStageBuilder`, and `EndStage()` returns to the `RenderPlanBuilder`.

`Build()` validates what can be known without views: every feature exists in the catalog, stage names are unique, and every view stage has a feature writing `$Color`.

`Build()` also flattens each frame feature and each stage once into a block of resolved passes (shaders reflected, bindless slots assigned, pipeline descriptions built). Nothing in a block depends on which views exist, so this work is never repeated when the set of views changes.

`SetRenderPlan()` replaces `SetRenderProgram()`. Setting a new plan clears the program cache.

### Views

A view is a persistent object, created by the renderer and held by the caller as a `Ref<RenderView>`. The stage is part of what the view is, so it is given at creation. The initial state of the view can be given at creation as well, and changed on the view afterwards.

```c++
Ref<RenderView> pGameView   = pRenderer->CreateView({.DebugName = "Game", .Stage = "Scene", .pWorld = &m_World});
Ref<RenderView> pPortalView = pRenderer->CreateView({.DebugName = "Portal", .Stage = "Portal", .pWorld = &m_World, .Target = portalTexture});

pGameView->SetCamera(m_pCamera->GetRenderCamera());
```

`RenderViewDesc::Stage` is a name. It is resolved against the active render plan, and resolved again when the plan is replaced.

The existing `RenderView` struct, which is what a pass sees of a view during execution, is renamed `RenderViewContext`.

State set on a view stays until it is changed, nothing has to be set again every frame:

- **World:** from the description, `SetWorld()` to change it.
- **Camera:** `SetCamera()` with a `RenderCamera`. The view does not look for a camera on its own, and never queries the world or its systems for one. Until a camera is set the view renders from the origin of the world. The camera is set again when it moves, e.g. from `CameraSystem::GetPrimaryCamera()` or the free camera of the editor.
- **Target and rect:** from the description, `SetTarget()` / `SetRect()` to change them.
- **Resources:** `GetResources()` is the view's `RenderResourceTable`, supplying `View` scoped resources the renderer does not provide itself.

The view also owns the GPU side state that has to survive between frames and between program switches:

- its camera uniform buffer (`view.camera`), replacing `WindowContext::CameraBufferHandle`
- its `View` scoped transient resources, such as `$Depth` and later history buffers

When the last reference to a view is released, its resources are retired once no frame in flight uses them. Cached programs do not refer to specific views (see Building and caching), so nothing else has to be cleaned up.

### Reading another view

What a view stage reads is declared by its passes, as for any other pass: globals, imports and exports. There is no separate view input concept.

The one thing a pass cannot declare is *which* view it reads, since views do not exist when the pass is written. That is bound through the resource table of the reading view:

```c++
// In the catalog: the scene pass reads a view scoped texture
catalog.RegisterResource("portal.color").WithType(EResourceType::SampledImage).WithScope(EResourceScope::View);
catalog.RegisterPass("pbr")....MapGlobal("portal.color", "portalTexture");

// On the view: bind the name to the texture the portal view renders to
pGameView->GetResources().Set("portal.color", portalTexture);
```

When building the program, a texture supplied through a resource table that is also the target of a view submitted this frame is resolved to that target instead of being treated as an external texture. The read then depends on the write in the merged program, and ordering and barriers follow from the existing DAG build and synchronization planning.

If the portal view is not submitted, the texture is an ordinary external texture with the content it was last rendered with. Nothing has to be declared optional.

### Submitting

```c++
pRenderer->SubmitView(pGameView);
```

A submit only says "render this view this frame". Submits live for one frame, and a view can be submitted at most once per frame. A view whose stage is not in the active plan, or which has no world, is rejected with an error.

`RenderRequest` is removed, its contents now live on the view.

### Targets

- **No target set:** the view writes the window's shared `$Color`. Several views can share it, each with its own viewport rect. The existing load op inference still applies: the first write of the frame clears, later ones load.
- **Target set:** the view writes its own offscreen texture, which can then be sampled by another view or by the UI.

`$Color` therefore resolves per target and not per view: views sharing a target share the resolved name, views with different targets do not. Frame features always write the window's `$Color`.

The final state of a target is derived instead of declared: `Present` for the window's backbuffer, `ShaderResource` for an offscreen target.

The aspect ratio of the camera comes from the viewport rect of the view, as it comes from the window today.

### Ordering

1. Frame features and view stages run in the order they were added to the plan.
2. Within a stage, a view reading the target of another view of the same stage is ordered after it.
3. Remaining ties are broken by submit order.

Reading the target of a view in a later stage, or a cycle within a stage, is a build error.

### Name scoping

Resolved names are global today (`$Color`, `$Depth`, `view.camera`, `scene.*`), so two views in the same program would share depth and camera. Resources get a scope when registered:

```c++
catalog.RegisterResource(Renderer::CAMERA_RESOURCE_NAME).WithType(EResourceType::UniformBuffer).WithScope(EResourceScope::View);
catalog.RegisterResource(RenderSystem::VERTICES_RESOURCE_NAME).WithScope(EResourceScope::World);
```

| Scope    | Resolved name               | Supplied by                              |
| -------- | --------------------------- | ---------------------------------------- |
| `Global` | `name`                      | `Renderer::GetGlobalResources()`         |
| `World`  | `world#<index>.name`        | The world's `RenderResourceTable`        |
| `View`   | `view#<slot>.name`          | The view: by the renderer (`view.camera`) or through `RenderView::GetResources()` |

The slot of a view is its position among the views rendered this frame, the index of a world the position of the first view rendering it. They say which views share what, not which view or world it is. The scope also decides which table an external resource is looked up in, there is no fallback between the tables.

`$Depth` and `$Stencil` are always `View` scoped, `$Color` is scoped to the target (`$Color` for the window, `target#<slot>.$Color` for the offscreen target first used by the view in that slot). Imports and exports between the features of a stage are `View` scoped as well, so the post process of one view never reads the geometry output of another. The prefix is added when a stage block is instantiated for a view. Pass declarations and shaders keep using the unscoped names.

A frame feature can only use `Global` resources and the window's `$Color`. Using a `View` or `World` scoped resource from a frame feature is a build error.

### Building and caching

The program key (`ProgramKey`) describes structure only. It is the ordered list of the views rendered this frame, each described by a `ProgramViewDesc`:

- the view stage it belongs to
- the slot of the first view rendering to the same target (0 for the window)
- the index of the first view rendering the same world
- the view scoped resources it has bound to the target of an earlier view

Nothing in the key identifies a specific view, world or texture. Two different views of the same stage produce the same key, and so do a view before and after its target was recreated at another size. The frame features and the stage contents are fixed by the plan and not part of the key. Camera values and target sizes are not part of it either.

```c++
// Inside the renderer render call
std::vector<RenderView*> views   = OrderSubmittedViews();
CachedProgram&           program = GetOrBuildProgram(BuildProgramKey(views)); // RenderProgramBuilder::Build() on a miss
```

`RenderProgramBuilder::Build()` walks the plan in order. A frame feature block is appended as it is. A stage block is appended once per view of the stage, with its names prefixed for the slot of the view and the index of its world. The result then goes through the existing `BuildDAG()`, `TopoSortALAP()` and `PlanSynchronization()`.

Two changes were needed in those steps: every color target is a sink of the graph (not only the last pass writing `$Color`), and a resource can have several passes waiting for its producer (two views reading the target of a third).

The cache holds the program together with its `RenderProgramInstance` and is LRU bounded (16 programs).

### Execution

`RenderViewContext` is per view, not per `Execute()` call. Each `ResolvedPass` records which view it belongs to, and the `ExecuteContext` of a pass is created with that view, so `ctx.GetWorld()` and `ctx.GetView()` keep working unchanged in pass code. Viewport and scissor are set from the view's rect. Passes of a frame feature get a context without view and world, covering the whole window.

### Keeping switches cheap

`RenderProgramInstance` owns pipelines, layouts and transient resources today, so replacing it rebuilds all of them. That state moves out:

- **Pipelines and layouts:** renderer level `PipelineCache` keyed by pass name and attachment formats, cleared when the plan is replaced. Pipelines are created the first time a pass is rendered with a set of formats. Since the plan fixes the full set of passes they could be created up front when the plan is set, this is not done yet.
- **View transients:** owned by the view (`RenderTransientResources`), keyed by the unscoped resource name.
- **Instance:** left with what really is per program: command pools and buffers, queue sync points and frame-in-flight bookkeeping.

A view that appears and disappears between frames then costs a cache lookup and nothing else.

Retiring an instance has to wait for the frames in flight that still use it: the destructor of `RenderProgramInstance` waits for the sync points of the frames it submitted. Textures and buffers released by a view go through the deferred destruction of the `ResourceManager`.

## Open questions

- **Variation within a stage.** Every view of a stage renders the same features. An editor viewport that wants the scene without post processing needs its own stage. Is that enough, or should a view be able to switch off features of its stage?
- **One stage per view.** A view belongs to exactly one stage. Is there a case for the same view taking part in two (for example a depth prepass stage shared by all views, followed by a scene stage)?
- **Shared views.** A shadow map is rendered once and read by several views. The resource table covers the read side, but who creates and submits such a view: the application, or a feature that asks for it?
- **Textures sampled by the UI.** `ImGui::Image(handle)` is not visible to the graph. Plan order and the derived `ShaderResource` final state make the editor viewport work, but nothing validates it. Should the UI feature declare the view targets it reads?
- **Reading a view target that is not rendered this frame.** The texture is then an ordinary external texture, and the synchronization plan assumes the state the resource was registered with. It should be registered `WithInitialState(FResourceState::ShaderResource)`, which is the state a target is left in. Should this be implied for view scoped textures?
- **Late catalog registration.** `RenderSystem` and `LightSystem` register their resources in the constructor, and the stage blocks are now built in `RenderPlanBuilder::Build()`. Systems still have to be added before the plan is built, as before the render program is built today.
- **Reading the target of another view is untested.** The path is implemented (ordering, key, name resolution) but no shader reads a view scoped texture yet, the portal feature of the examples does not exist.
- **Frame feature naming.** `AddFrameFeature()` keeps "frame" since it describes when the feature runs (once per frame, not per view). Rename if "render" should be used throughout.
- **Debugging.** A dump of the composed frame (plan entries, views, passes, resolved names) would make ordering and scoping problems visible.

## Resolved while implementing

- **Depth for views sharing a target.** Depth is owned per view and sized to the target. The view renders to its rect through viewport and scissor.
- **Target resize.** The caller owns the target texture and recreates it, then calls `RenderView::SetTarget()`. This does not change the program key. The editor does this when the viewport panel changes size.
- **Multiple windows.** Only the first window added to the renderer is rendered to and presented.
- **Old entry points.** `RenderGraph::Begin()`, `Renderer::SetRenderProgram()`, `Renderer::Submit()` and `RenderRequest` are removed. `RenderProgramBuilder` is now the internal step compiling a plan and a key into a program.

## Implementation steps

1. Resource scopes: `WithScope()`, prefixing when flattening, per-pass view in `ResolvedPass` and `ExecuteContext`.
2. Views: `RenderView` object with world, camera and resource table, existing struct renamed `RenderViewContext`, camera buffer and view transients moved from `WindowContext` and `RenderProgramInstance` to the view. `SubmitView()` replaces `Submit()`.
3. Pipeline and layout cache at renderer level, deferred retire of instances.
4. Targets: optional offscreen target, target scoped `$Color`, derived final states, viewport rect.
5. `RenderPlan`: builder, frame features, view stages, prebuilt blocks, `SetRenderPlan()` replacing `SetRenderProgram()`.
6. Program key and cache, `BuildProgram()` from plan and submitted views.
7. Resolving resource table textures to view targets, ordering between views.
8. Sandbox and editor moved over: one game view in both, offscreen viewport in the editor.
