#include "polypch.h"

#include "PVKSemaphore.h"
#include "PVKInstance.h"

namespace Poly
{
	PVKSemaphore::~PVKSemaphore()
	{
		// TODO: Make sure semaphore isn't being used at the moment
		// Alternative: be clear that the user needs to handle that
		vkDestroySemaphore(PVKInstance::GetDevice(), m_Semaphore, nullptr);
		m_Semaphore = VK_NULL_HANDLE;
	}

	void PVKSemaphore::Init()
	{
		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		createInfo.flags = 0;
		createInfo.pNext = nullptr;

		PVK_CHECK(vkCreateSemaphore(PVKInstance::GetDevice(), &createInfo, nullptr, &m_Semaphore), "Failed to create semaphore");
	}

	void PVKSemaphore::AddWaitStageMask(FPipelineStage stage)
	{
		m_WaitStage |= stage;
	}
}