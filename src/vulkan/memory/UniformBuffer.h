#pragma once

#include "Buffer.h"
#include "renderer/types/UniformBufferData.h"

class VulkanUniformBuffer : public VulkanBuffer
{
public:
	void create(VulkanDevice& device)
	{
		vk::DeviceSize size = sizeof(UniformBufferData);
		VulkanBuffer::create(device, size, vk::BufferUsageFlagBits::eUniformBuffer);
	}
};