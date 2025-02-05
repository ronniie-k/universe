#include "Renderer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <chrono>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "vulkan/Device.h"
#include "vulkan/memory/UniformBuffer.h"
#include "vulkan/pipeline/GraphicsPipeline.h"
#include "vulkan/swapchain/Swapchain.h"
#include "vulkan/Utils.h"
#include "types/Vertex.h"

namespace
{
const std::vector<Vertex> vertices = { { { -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f } },
									   { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
									   { { 0.5f, 0.5f }, { 0.0f, 1.0f, 1.0f } },
									   { { -0.5f, 0.5f }, { 1.0f, 0.0f, 1.0f } } };
const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };
} // namespace

Renderer::Renderer()
{
}

void Renderer::initVulkan(Window* window)
{
	m_window = window;
	auto* glfwWindow = m_window->getGLFWWindow();

	m_instance.create();
	m_swapchain.createSurface(m_instance.handle, glfwWindow);
	m_device.create(m_instance.handle, m_swapchain.getSurface());
	m_swapchain.create(m_device, glfwWindow);

	createCommandObjects();
	createSyncObjects();

	for (int i = 0; i < m_framesInFlight; i++)
	{
		VulkanUniformBuffer buffer;
		buffer.create(m_device);
		m_uniformBuffers.emplace_back(std::move(buffer));
	}

	m_vertexBuffer.create(m_device, vertices);
	m_indexBuffer.create(m_device, indices);

	m_pipelineDescriptor.create(m_device.handle, m_framesInFlight, m_uniformBuffers);

	m_pipeline.create(m_device.handle, m_swapchain, "vert.spv", "frag.spv", m_pipelineDescriptor.getLayout());
	m_swapchain.createFramebuffers(m_pipeline.getRenderPass());
}

void Renderer::createCommandObjects()
{
	vulkan_utils::QueueFamilyIndices queueFamilies =
		vulkan_utils::findQueueFamilies(m_device.getPhysicalDevice(), m_swapchain.getSurface());

	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	createInfo.setQueueFamilyIndex(queueFamilies.graphicsFamily.value());

	m_commandPool = m_device.handle.createCommandPool(createInfo);

	m_commandBuffers.resize(m_framesInFlight);

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(m_commandPool);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	allocInfo.setCommandBufferCount(m_commandBuffers.size());

	m_commandBuffers = m_device.handle.allocateCommandBuffers(allocInfo);
}


void Renderer::createSyncObjects()
{
	m_imgAvailableSemaphores.resize(m_framesInFlight);
	m_renderFinishedSemaphores.resize(m_framesInFlight);
	m_inFlightFences.resize(m_framesInFlight);

	for (int i = 0; i < m_framesInFlight; i++)
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo;
		m_imgAvailableSemaphores[i] = m_device.handle.createSemaphore(semaphoreCreateInfo);
		m_renderFinishedSemaphores[i] = m_device.handle.createSemaphore(semaphoreCreateInfo);

		vk::FenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
		m_inFlightFences[i] = m_device.handle.createFence(fenceCreateInfo);
	}
}

void Renderer::recordCommandBuffer(vk::CommandBuffer cmdBuffer, uint32_t imgIndex)
{
	vk::CommandBufferBeginInfo info;
	cmdBuffer.begin(info);

	vk::ClearValue clearColor = vk::ClearColorValue { 0.f, 0.f, 0.f, 1.f };

	vk::RenderPassBeginInfo renderPassInfo;
	renderPassInfo.setRenderPass(m_pipeline.getRenderPass());
	renderPassInfo.setFramebuffer(m_swapchain.getFramebuffer(imgIndex));
	renderPassInfo.renderArea.offset = vk::Offset2D { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchain.getExtent();
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	auto extent = m_swapchain.getExtent();
	vk::Viewport viewport;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vk::Rect2D scissor {};
	scissor.offset = vk::Offset2D { 0, 0 };
	scissor.extent = extent;

	std::array<vk::Buffer, 1> vertexBuffers = { m_vertexBuffer.handle };
	std::array<vk::DeviceSize, 1> offsets = { 0 };

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.handle);
	cmdBuffer.setViewport(0, 1, &viewport);
	cmdBuffer.setScissor(0, 1, &scissor);
	cmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
	cmdBuffer.bindIndexBuffer(m_indexBuffer.handle, 0, vk::IndexType::eUint16);
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline.getLayout(), 0, 1,
								 &m_pipelineDescriptor.getDescriptorSet(m_currentFrame), 0, nullptr);
	cmdBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
	cmdBuffer.draw(3, 1, 0, 0);

	cmdBuffer.endRenderPass();
	cmdBuffer.end();
}

void Renderer::drawFrame()
{
	VulkanSwapchain& swapchain = m_swapchain;

	(void) m_device.handle.waitForFences(1, &m_inFlightFences[m_currentFrame], true, UINT64_MAX);

	auto nextImgResult = m_device.handle.acquireNextImageKHR(swapchain.handle, UINT64_MAX, m_imgAvailableSemaphores[m_currentFrame]);
	uint32_t imgIndex = nextImgResult.value;

	if (nextImgResult.result == vk::Result::eErrorOutOfDateKHR)
	{
		swapchain.recreate(m_window->getGLFWWindow());
		return;
	}
	else if (nextImgResult.result != vk::Result::eSuccess && nextImgResult.result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swapchain img");
	}

	(void) m_device.handle.resetFences(1, &m_inFlightFences[m_currentFrame]);

	updateUniformBuffer();

	m_commandBuffers[m_currentFrame].reset();
	recordCommandBuffer(m_commandBuffers[m_currentFrame], imgIndex);

	std::array<vk::Semaphore, 1> waitSemaphores = { m_imgAvailableSemaphores[m_currentFrame] };
	std::array<vk::Semaphore, 1> signalSemaphores = { m_renderFinishedSemaphores[m_currentFrame] };
	std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::array<vk::CommandBuffer, 1> cmdBuffers = { m_commandBuffers[m_currentFrame] };
	vk::SubmitInfo submitInfo;
	submitInfo.setWaitSemaphores(waitSemaphores);
	submitInfo.setSignalSemaphores(signalSemaphores);
	submitInfo.setWaitDstStageMask(waitStages);
	submitInfo.setCommandBuffers(cmdBuffers);

	(void) m_device.getGraphicsQueue().submit(1, &submitInfo, m_inFlightFences[m_currentFrame]);

	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphores(signalSemaphores);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain.handle;
	presentInfo.pImageIndices = &imgIndex;

	vk::Result result = m_device.getPresentQueue().presentKHR(presentInfo);

	if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_window->hasResized())
	{
		m_window->setResized(false);
		swapchain.recreate(m_window->getGLFWWindow());
	}
	else if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to present swapchain img");
	}

	m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
}

void Renderer::updateUniformBuffer()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	auto swapchianExtent = m_swapchain.getExtent();

	UniformBufferData ubo {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	ubo.proj = glm::perspective(glm::radians(45.0f), swapchianExtent.width / (float) swapchianExtent.height, 0.1f, 10.0f);

	ubo.proj[1][1] *= -1;

	m_uniformBuffers[m_currentFrame].copyData(&ubo, sizeof(UniformBufferData));
}

void Renderer::waitIdle()
{
	m_device.handle.waitIdle();
}
