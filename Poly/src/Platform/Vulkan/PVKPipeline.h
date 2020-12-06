#pragma once
#include <vulkan/vulkan.h>
#include <string>

namespace Poly
{
	class PVKSwapChain;
	class PVKShader;
	class PVKRenderPass;
	class PVKDescriptor;

	class PVKPipeline
	{
	public:
		PVKPipeline();
		~PVKPipeline();

		void Init(PVKSwapChain& swapChain, PVKShader& shader, PVKRenderPass& renderPass);
		void Cleanup();

		void AddVertexDescriptions(uint32_t binding, uint32_t location, uint32_t stride, VkFormat format);
		void SetDescriptor(PVKDescriptor& descriptor);
		VkPipelineBindPoint GetType() const { return m_PipelineType; }
		VkPipeline GetNative() const { return m_Pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

	private:
		void CreatePipeline();

		VkPipelineBindPoint	m_PipelineType		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		VkPipeline			m_Pipeline			= VK_NULL_HANDLE;
		VkPipelineLayout	m_PipelineLayout	= VK_NULL_HANDLE;
		PVKShader*			m_pShader			= nullptr;
		PVKDescriptor*		m_pDescriptor		= nullptr;
		std::vector<VkVertexInputBindingDescription> m_VertexBinding;
		std::vector<VkVertexInputAttributeDescription> m_VertexAttributes;

		PVKRenderPass*	m_pRenderPass	= nullptr;
		PVKSwapChain*	m_pSwapChain	= nullptr;
	};

}