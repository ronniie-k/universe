#pragma once

#include "vulkan/Instance.h"
#include "vulkan/Window.h"
#include "vulkan/pipeline/GraphicsPipeline.h"
#include <cstdint>
#include <memory>

class Renderer
{
public:
	Renderer();

	void initVulkan(Window* window);
	void cleanup();

	void createPipeline(const std::string& vertexSPV, const std::string& fragSPV);
	void createCommandObjects();
	void createSyncObjects();

	void recordCommandBuffer(vk::CommandBuffer cmdBuffer, uint32_t imgIndex);

	void drawFrame();
	void waitIdle();

private:
	VulkanInstance m_instance;
	VulkanGraphicsPipeline m_pipeline;
	Window* m_window = nullptr;

	const uint32_t m_framesInFlight = 2;
	uint32_t m_currentFrame = 0;

	vk::CommandPool m_commandPool;
	std::vector<vk::CommandBuffer> m_commandBuffers;

	std::vector<vk::Semaphore> m_imgAvailableSemaphores;
	std::vector<vk::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::Fence> m_inFlightFences;
};