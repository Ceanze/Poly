#include "polypch.h"
#include "PVKCommandPool.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"

namespace Poly
{

	PVKCommandPool::PVKCommandPool() :
		commandPool(VK_NULL_HANDLE), instance(nullptr)
	{
	}

	PVKCommandPool::~PVKCommandPool()
	{
	}

	void PVKCommandPool::init(PVKInstance* instance)
	{
		this->instance = instance;
		
		createCommandPool();
	}

	void PVKCommandPool::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(this->instance->getPhysicalDevice());

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; // Optional

		PVK_CHECK(vkCreateCommandPool(this->instance->getDevice(), &poolInfo, nullptr, &this->commandPool), "Failed to create command pool!");
	}

}