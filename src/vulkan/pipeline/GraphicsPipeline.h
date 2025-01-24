#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

class VulkanDevice;

class VulkanGraphicsPipeline
{
public:
	void create(vk::Device device,
				const std::string& vertexSPV,
				const std::string& fragSPV,
				vk::Format swapchainFormat,
				vk::Extent2D swapchainExtent);
	void destroy();

	vk::RenderPass getRenderPass() { return m_renderPass; }

	vk::Pipeline handle;

private:
	vk::PipelineShaderStageCreateInfo createShaderStage(const std::string& shaderSPV, vk::ShaderStageFlagBits stage);
	void createRenderPass(vk::Format swapchainFormat);
	void createPipeline(const std::string& vertexSPV, const std::string& fragSPV, vk::Extent2D swapchainExtent);

private:
	vk::Device m_device;

	vk::PipelineLayout m_layout;
	vk::RenderPass m_renderPass;
	std::vector<vk::ShaderModule> m_cachedShaderModules;
};