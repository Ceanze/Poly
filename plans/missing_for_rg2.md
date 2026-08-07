# Currently missing things to make RG2 feature complete with integration

## No Scene→GPU bridge for the new system

> Resolved by SceneRenderBridge (Poly/RenderGraph/SceneRenderBridge.h/.cpp)

The old Scene::CreateRenderScene(RenderGraphProgram&) / RenderScene (RenderScene.h) batches meshes into vertex/instance/material buffers, but it's hard-coupled to the old RenderGraphProgram/ResID/PassResID types. SceneRenderBridge is the RG2 equivalent, wired via Scene::CreateSceneRenderBridge()/GetSceneRenderBridge() alongside (not replacing) the RG1 path. Unlike RenderScene it concatenates all meshes into one combined vertex/index buffer instead of binding each mesh's own buffer per draw - per-draw variation is expressed through DrawIndexedInstanced's baseVertex/firstIndex/firstInstance instead. Still open: no PBR pass is actually registered against RG2 yet (SandboxApp is still fully RG1) - that's blocked on the Renderer accessor below.

## Texture-per-material vs. one-texture-set-per-pass mismatch

> Resolved by SceneRenderBridge - moved into the MaterialValues buffer

pbr_bindless.frag used to read all textures from pc.textureIndices[0..5] — one fixed set for the whole pass/draw, built once per pass before ExecuteFn runs (RenderProgramInstance.cpp:585-595), not once per draw call. Per-material texture indices now live in MaterialValues.TextureIndices[6] (GPUMaterialData in SceneRenderBridge.h), indexed per-draw via in_MaterialIndex the same way the scalar PBR values already were - no push-constant repacking between draws needed.

## Renderer exposes no accessor to the live RenderProgramInstance

> Medium prio (needed, but simple)

SandboxApp needs to call UpdateResource() for camera/lights/scene buffers and textures, but Renderer::WindowContext::pRenderProgramInstance is private with no getter — there's currently no way for app code to reach the instance it just set via SetRenderProgram.
Exposing setters via the renderer can also be acceptable

## No per-frame "update these bytes" convenience

> Low prio (not needed, but great improvement)

The old RenderGraphProgram::UpdateGraphResource(name, size, data) copied CPU data into an already-owned GPU buffer. The new RenderProgramInstance::UpdateResource(name, Ref<Buffer>) instead swaps in a whole new Buffer ref. For a per-frame value like the camera matrix, SandboxApp would now need to own a persistently-mapped Ref<Buffer> itself (created with FBufferUsage::SHADER_DEVICE_ADDRESS, per plans/bindless.md), write into it via Buffer::TransferData, and call UpdateResource once to register it — that ownership/lifetime pattern doesn't exist anywhere yet.