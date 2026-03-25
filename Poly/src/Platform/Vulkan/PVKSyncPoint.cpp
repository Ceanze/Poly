#include "PVKSyncPoint.h"

#include "polypch.h"
#include "PVKInstance.h"

namespace Poly
{
	PVKSyncPoint::~PVKSyncPoint()
	{
		// TODO: Make sure semaphore isn't being used at the moment
		// Alternative: be clear that the user needs to handle that
		vkDestroySemaphore(PVKInstance::GetDevice(), m_TimelineSemaphore, nullptr);
		m_TimelineSemaphore = VK_NULL_HANDLE;
	}

	void PVKSyncPoint::Init()
	{
		VkSemaphoreTypeCreateInfo typeCreateInfo = {};
		typeCreateInfo.sType                     = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		typeCreateInfo.semaphoreType             = VK_SEMAPHORE_TYPE_TIMELINE;
		typeCreateInfo.initialValue              = 0;
		typeCreateInfo.pNext                     = nullptr;

		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.flags                 = 0;
		createInfo.pNext                 = &typeCreateInfo;

		PVK_CHECK(vkCreateSemaphore(PVKInstance::GetDevice(), &createInfo, nullptr, &m_TimelineSemaphore), "Failed to create semaphore");
	}

	void PVKSyncPoint::AddWaitStageMask(FPipelineStage stage)
	{
		m_WaitStage |= stage;
	}

	void PVKSyncPoint::Wait(uint64 value)
	{
		VkSemaphoreWaitInfo waitInfo;
		waitInfo.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.pNext          = NULL;
		waitInfo.flags          = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores    = &m_TimelineSemaphore;
		waitInfo.pValues        = &value;

		vkWaitSemaphores(PVKInstance::GetDevice(), &waitInfo, UINT64_MAX);
	}

	void PVKSyncPoint::Signal(uint64 value)
	{
		VkSemaphoreSignalInfo signalInfo;
		signalInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		signalInfo.pNext     = nullptr;
		signalInfo.semaphore = m_TimelineSemaphore;
		signalInfo.value     = value;

		vkSignalSemaphore(PVKInstance::GetDevice(), &signalInfo);
	}

	uint64 PVKSyncPoint::GetValue() const
	{
		uint64_t value;
		vkGetSemaphoreCounterValue(PVKInstance::GetDevice(), m_TimelineSemaphore, &value);

		return value;
	}
} // namespace Poly