#include "polypch.h"
#include "PVKFence.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKFence::~PVKFence()
	{
		// TODO: Make sure it is safe to delete
		vkDestroyFence(PVKInstance::GetDevice(), m_Fence, nullptr);
	}

	void PVKFence::Init(FFenceFlag fenceFlag)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		PVK_CHECK(vkCreateFence(PVKInstance::GetDevice(), &fenceCreateInfo, nullptr, &m_Fence), "Failed to create fence!");
	}

	void PVKFence::Reset()
	{
		vkResetFences(PVKInstance::GetDevice(), 1, &m_Fence);
	}

	void PVKFence::Wait(uint64 timeout)
	{
		vkWaitForFences(PVKInstance::GetDevice(), 1, &m_Fence, VK_TRUE, timeout);
	}
}