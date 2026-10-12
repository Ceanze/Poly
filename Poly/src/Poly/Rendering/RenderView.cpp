#include "RenderView.h"

#include "Poly/RenderGraph/Shader/GPUCameraData.h"
#include "Poly/Rendering/Renderer.h"

namespace Poly
{
	RenderView::RenderView(RenderViewDesc desc)
	    : m_Desc(std::move(desc))
	{
		m_CameraBufferHandle = ResourceManager::CreateUniformBuffer(sizeof(GPUCameraData), "RenderView.Camera(" + m_Desc.DebugName + ")");
	}

	RenderView::~RenderView()
	{
		// Destruction is deferred by the resource manager until no frame in flight uses the buffer,
		// the same goes for the transient resources of the view
		ResourceManager::Destroy(m_CameraBufferHandle);
	}

	void RenderView::PrepareFrame(uint32 targetWidth, uint32 targetHeight)
	{
		m_FrameResources = m_Resources;

		// The application provides its own camera buffer
		if (m_FrameResources.GetEntries().contains(Renderer::CAMERA_RESOURCE_NAME))
			return;

		const bool   hasRect = m_Desc.Rect.Width != 0 && m_Desc.Rect.Height != 0;
		const uint32 width   = hasRect ? m_Desc.Rect.Width : targetWidth;
		const uint32 height  = hasRect ? m_Desc.Rect.Height : targetHeight;
		const float  aspect  = height != 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;

		glm::mat4 projection = glm::perspective(m_Camera.FieldOfView, aspect, m_Camera.NearPlane, m_Camera.FarPlane);
		projection[1][1] *= -1.0f;

		const GPUCameraData cameraData = {.ViewProjection = projection * m_Camera.View, .Position = glm::vec4(m_Camera.Position, 1.0f)};
		ResourceManager::UploadBufferData(m_CameraBufferHandle, &cameraData, sizeof(GPUCameraData));
		m_FrameResources.Set(Renderer::CAMERA_RESOURCE_NAME, m_CameraBufferHandle);
	}
} // namespace Poly
