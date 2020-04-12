#pragma once

#include <stdint.h>

namespace Poly
{

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

		virtual void init(uint32_t width, uint32_t height) = 0;
		virtual void beginScene() = 0;
		virtual void setActiveCamera(Camera* camera) = 0;
		virtual void draw(Model* model) = 0; // More draws will be created in the future as overloads
		virtual void endScene() = 0;
		virtual void shutdown() = 0;

		virtual void createRenderer(/*Renderer::MESH*/) = 0;
		virtual VertexBuffer* createVertexBuffer() = 0;
		virtual IndexBuffer* createIndexBuffer() = 0;
		virtual UniformBuffer* createUniformBuffer() = 0;
		virtual StorageBuffer* createStorageBuffer() = 0;
		virtual Texture* createTexture() = 0;
	};

}