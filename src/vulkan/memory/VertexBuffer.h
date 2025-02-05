#pragma once

#include "Buffer.h"
#include "vulkan/Device.h"
#include "renderer/types/Vertex.h"

#include <vector>
#include <vulkan/vulkan.hpp>

class VulkanVertexBuffer : public VulkanBuffer
{
public:
	void create(VulkanDevice& device, const std::vector<Vertex>& vertices)
	{
		vk::DeviceSize size = sizeof(vertices[0]) * vertices.size();
		VulkanBuffer::create(device, size, vk::BufferUsageFlagBits::eVertexBuffer);
		copyData(vertices.data(), size);
	}
};