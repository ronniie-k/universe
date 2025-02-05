#pragma once

#include <cstdint>

#include "vulkan/Instance.h"
#include "vulkan/Window.h"
#include "vulkan/memory/IndexBuffer.h"
#include "vulkan/memory/UniformBuffer.h"
#include "vulkan/pipeline/GraphicsPipeline.h"
#include "vulkan/memory/VertexBuffer.h"
#include "vulkan/pipeline/PipelineDescriptor.h"

class Renderer
{
public:
	Renderer();

	void initVulkan(Window* window);
	void cleanup();

	void createCommandObjects();
	void createSyncObjects();

	void recordCommandBuffer(vk::CommandBuffer cmdBuffer, uint32_t imgIndex);

	void drawFrame();
	void waitIdle();

private:
	void updateUniformBuffer();

private:
	VulkanInstance m_instance;
	VulkanDevice m_device;
	VulkanSwapchain m_swapchain;

	PipelineDescriptor m_pipelineDescriptor;
	VulkanGraphicsPipeline m_pipeline;
	Window* m_window = nullptr;

	const uint32_t m_framesInFlight = 2;
	uint32_t m_currentFrame = 0;

	vk::CommandPool m_commandPool;
	std::vector<vk::CommandBuffer> m_commandBuffers;

	std::vector<vk::Semaphore> m_imgAvailableSemaphores;
	std::vector<vk::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::Fence> m_inFlightFences;

	VulkanVertexBuffer m_vertexBuffer;
	VulkanIndexBuffer m_indexBuffer;
	std::vector<VulkanUniformBuffer> m_uniformBuffers;
};