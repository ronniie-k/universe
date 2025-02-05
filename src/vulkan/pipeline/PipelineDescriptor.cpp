#include "PipelineDescriptor.h"
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>


void PipelineDescriptor::create(vk::Device device, uint32_t framesInFlight, const std::vector<VulkanUniformBuffer>& ubos)
{
	m_device = device;
	createDescriptorSetLayout();
	createDescriptorPool(framesInFlight);
	createDescriptorSets(framesInFlight, ubos);
}

void PipelineDescriptor::destroy()
{
	m_device.destroyDescriptorPool(m_descriptorPool);
	m_device.destroyDescriptorSetLayout(m_layout);
}

void PipelineDescriptor::createDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding layoutBinding;
	layoutBinding.setBinding(0);
	layoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
	layoutBinding.setDescriptorCount(1);
	layoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setBindingCount(1);
	createInfo.setPBindings(&layoutBinding);

	m_layout = m_device.createDescriptorSetLayout(createInfo);
}

void PipelineDescriptor::createDescriptorPool(uint32_t framesInFlight)
{
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer);
	poolSize.setDescriptorCount(framesInFlight);

	vk::DescriptorPoolCreateInfo createInfo;
	createInfo.setPoolSizeCount(1);
	createInfo.setPPoolSizes(&poolSize);
	createInfo.setMaxSets(framesInFlight);

	m_descriptorPool = m_device.createDescriptorPool(createInfo);
}

void PipelineDescriptor::createDescriptorSets(uint32_t framesInFlight, const std::vector<VulkanUniformBuffer>& ubos)
{
	std::vector<vk::DescriptorSetLayout> layouts(framesInFlight, m_layout);

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(m_descriptorPool);
	allocInfo.setDescriptorSetCount(framesInFlight);
	allocInfo.setSetLayouts(layouts);

	m_descriptorSets = m_device.allocateDescriptorSets(allocInfo);

	for (int i = 0; i < framesInFlight; i++)
	{
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.setBuffer(ubos[i].handle);
		bufferInfo.setOffset(0);
		bufferInfo.setRange(sizeof(UniformBufferData));

		vk::WriteDescriptorSet write;
		write.setDstSet(m_descriptorSets[i]);
		write.setDstBinding(0);
		write.setDstArrayElement(0);
		write.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		write.setDescriptorCount(1);
		write.setPBufferInfo(&bufferInfo);

		m_device.updateDescriptorSets(1, &write, 0, nullptr);
	}
}
