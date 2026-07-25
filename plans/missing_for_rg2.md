# Currently missing things to make RG2 feature complete with integration

## No Scene→GPU bridge for the new system

> High prio

The old Scene::CreateRenderScene(RenderGraphProgram&) / RenderScene (RenderScene.h) batches meshes into vertex/instance/material buffers, but it's hard-coupled to the old RenderGraphProgram/ResID/PassResID types. There is no equivalent that builds a combined vertex buffer, per-instance transform buffer, and material-properties buffer matching pbr_bindless.frag's Vertex/MaterialValues layouts, nor anything that calls RenderProgramInstance::UpdateResource() with them.

## Texture-per-material vs. one-texture-set-per-pass mismatch

> High prio (indexed drawing handling for passes)

pbr_bindless.frag reads all textures from pc.textureIndices[0..5] — one fixed set for the whole pass/draw. But RecordPass builds and uploads push constants exactly once per pass, before ExecuteFn runs (RenderProgramInstance.cpp:585-595), not once per draw call. A multi-material scene like sponza can't get correct per-submesh textures this way unless texture indices move into the (already-indexed-by-gl_InstanceIndex) MaterialValues buffer instead of the push constant, or ExecuteFn gets a way to repack/repush constants between draws.

## Renderer exposes no accessor to the live RenderProgramInstance

> Medium prio (needed, but simple)

SandboxApp needs to call UpdateResource() for camera/lights/scene buffers and textures, but Renderer::WindowContext::pRenderProgramInstance is private with no getter — there's currently no way for app code to reach the instance it just set via SetRenderProgram.
Exposing setters via the renderer can also be acceptable

## No per-frame "update these bytes" convenience

> Low prio (not needed, but great improvement)

The old RenderGraphProgram::UpdateGraphResource(name, size, data) copied CPU data into an already-owned GPU buffer. The new RenderProgramInstance::UpdateResource(name, Ref<Buffer>) instead swaps in a whole new Buffer ref. For a per-frame value like the camera matrix, SandboxApp would now need to own a persistently-mapped Ref<Buffer> itself (created with FBufferUsage::SHADER_DEVICE_ADDRESS, per plans/bindless.md), write into it via Buffer::TransferData, and call UpdateResource once to register it — that ownership/lifetime pattern doesn't exist anywhere yet.