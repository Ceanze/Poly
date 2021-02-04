#pragma once

/*
	Interface for platform renderers, handles the camera in the interface
*/

#include <stdint.h>

namespace Poly
{
	enum class Renderer { TEST, MESH };

	class VertexBuffer;
	class IndexBuffer;
	class UniformBuffer;
	class StorageBuffer;
	class Texture;
	class Model;
	class Camera;

	class IPlatformRenderer
	{
	public:
		IPlatformRenderer() = default;
		virtual ~IPlatformRenderer() = default;

		virtual void Init(uint32_t width, uint32_t height) = 0;
		virtual void BeginScene() = 0;
		virtual void SetActiveCamera(Camera* pCamera) { m_pCamera = pCamera; }
		virtual void Draw(Model* pModel) = 0; // More draws will be created in the future as overloads
		virtual void EndScene() = 0;

		virtual void CreateRenderer(Renderer subRenderer) = 0;
		virtual VertexBuffer* CreateVertexBuffer() = 0;
		virtual IndexBuffer* CreateIndexBuffer() = 0;
		virtual UniformBuffer* CreateUniformBuffer() = 0;
		virtual StorageBuffer* CreateStorageBuffer() = 0;
		virtual Texture* CreateTexture() = 0;

	protected:
		Camera* m_pCamera;
	};

}