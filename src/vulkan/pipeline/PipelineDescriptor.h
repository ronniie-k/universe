#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include "vulkan/memory/UniformBuffer.h"

class PipelineDescriptor
{
public:
	void create(vk::Device device, uint32_t framesInFlight, const std::vector<VulkanUniformBuffer>& ubos);

	void destroy();

	vk::DescriptorSetLayout getLayout() { return m_layout; }
	vk::DescriptorSet& getDescriptorSet(uint32_t index) { return m_descriptorSets[index]; }

private:
	void createDescriptorSetLayout();
	void createDescriptorPool(uint32_t framesInFlight);
	void createDescriptorSets(uint32_t framesInFlight, const std::vector<VulkanUniformBuffer>& ubos);

private:
	vk::DescriptorSetLayout m_layout;
	vk::DescriptorPool m_descriptorPool;
	std::vector<vk::DescriptorSet> m_descriptorSets;
	vk::Device m_device;
};