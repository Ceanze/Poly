#include "polypch.h"

#include "PVKBinarySemaphore.h"
#include "PVKInstance.h"

namespace Poly
{
	PVKBinarySemaphore::~PVKBinarySemaphore()
	{
		// TODO: Make sure semaphore isn't being used at the moment
		// Alternative: be clear that the user needs to handle that
		vkDestroySemaphore(PVKInstance::GetDevice(), m_Semaphore, nullptr);
		m_Semaphore = VK_NULL_HANDLE;
	}

	void PVKBinarySemaphore::Init()
	{
		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.flags = 0;
		createInfo.pNext = nullptr;

		PVK_CHECK(vkCreateSemaphore(PVKInstance::GetDevice(), &createInfo, nullptr, &m_Semaphore), "Failed to create semaphore");
	}

	void PVKBinarySemaphore::AddWaitStageMask(FPipelineStage stage)
	{
		m_WaitStage |= stage;
	}
}