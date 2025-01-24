#include "Renderer.h"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

#include "vulkan/Device.h"
#include "vulkan/pipeline/GraphicsPipeline.h"
#include "vulkan/swapchain/Swapchain.h"
#include "vulkan/Utils.h"

Renderer::Renderer()
{
}

void Renderer::cleanup()
{
	m_instance.getDevice().handle.destroyCommandPool(m_commandPool);
	VulkanDevice& device = m_instance.getDevice();
	for (int i = 0; i < m_framesInFlight; i++)
	{
		device.handle.destroySemaphore(m_imgAvailableSemaphores[i]);
		device.handle.destroySemaphore(m_renderFinishedSemaphores[i]);
		device.handle.destroyFence(m_inFlightFences[i]);
	}
	m_instance.destroy();
}

void Renderer::initVulkan(Window* window)
{
	m_window = window;
	m_instance.create(window);
	createCommandObjects();
	createSyncObjects();
	// vulkan_utils::printAvailableExtensions();
}

void Renderer::createPipeline(const std::string& vertexSPV, const std::string& fragSPV)
{
	auto& swapchain = m_instance.getSwapchain();
	m_pipeline.create(m_instance.getDevice().handle, vertexSPV, fragSPV, swapchain.getFormat(), swapchain.getExtent());
	swapchain.createFramebuffers(m_pipeline.getRenderPass());
}

void Renderer::createCommandObjects()
{
	VulkanDevice& device = m_instance.getDevice();
	// command pool
	vulkan_utils::QueueFamilyIndices queueFamilies =
		vulkan_utils::findQueueFamilies(device.getDevice(), m_instance.getSwapchain().getSurface());

	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	createInfo.setQueueFamilyIndex(queueFamilies.graphicsFamily.value());

	m_commandPool = device.handle.createCommandPool(createInfo);

	m_commandBuffers.resize(m_framesInFlight);

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(m_commandPool);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	allocInfo.setCommandBufferCount(m_commandBuffers.size());

	m_commandBuffers = device.handle.allocateCommandBuffers(allocInfo);
}


void Renderer::createSyncObjects()
{
	m_imgAvailableSemaphores.resize(m_framesInFlight);
	m_renderFinishedSemaphores.resize(m_framesInFlight);
	m_inFlightFences.resize(m_framesInFlight);

	for (int i = 0; i < m_framesInFlight; i++)
	{
		vk::SemaphoreCreateInfo semaphoreCreateInfo;
		m_imgAvailableSemaphores[i] = m_instance.getDevice().handle.createSemaphore(semaphoreCreateInfo);
		m_renderFinishedSemaphores[i] = m_instance.getDevice().handle.createSemaphore(semaphoreCreateInfo);

		vk::FenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
		m_inFlightFences[i] = m_instance.getDevice().handle.createFence(fenceCreateInfo);
	}
}

void Renderer::recordCommandBuffer(vk::CommandBuffer cmdBuffer, uint32_t imgIndex)
{
	VulkanSwapchain& swapchain = m_instance.getSwapchain();
	vk::CommandBufferBeginInfo info;
	cmdBuffer.begin(info);

	vk::ClearValue clearColor = vk::ClearColorValue { 0.f, 0.f, 0.f, 1.f };

	vk::RenderPassBeginInfo renderPassInfo;
	renderPassInfo.setRenderPass(m_pipeline.getRenderPass());
	renderPassInfo.setFramebuffer(swapchain.getFramebuffer(imgIndex));
	renderPassInfo.renderArea.offset = vk::Offset2D { 0, 0 };
	renderPassInfo.renderArea.extent = swapchain.getExtent();
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	auto extent = swapchain.getExtent();
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

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.handle);
	cmdBuffer.setViewport(0, 1, &viewport);
	cmdBuffer.setScissor(0, 1, &scissor);
	cmdBuffer.draw(3, 1, 0, 0);

	cmdBuffer.endRenderPass();
	cmdBuffer.end();
}

void Renderer::drawFrame()
{
	vk::Device device = m_instance.getDevice().handle;
	VulkanSwapchain& swapchain = m_instance.getSwapchain();

	(void) device.waitForFences(1, &m_inFlightFences[m_currentFrame], true, UINT64_MAX);

	auto nextImgResult = device.acquireNextImageKHR(swapchain.handle, UINT64_MAX, m_imgAvailableSemaphores[m_currentFrame]);
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

	(void) device.resetFences(1, &m_inFlightFences[m_currentFrame]);

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

	(void) m_instance.getDevice().getGraphicsQueue().submit(1, &submitInfo, m_inFlightFences[m_currentFrame]);

	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphores(signalSemaphores);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain.handle;
	presentInfo.pImageIndices = &imgIndex;

	vk::Result result = m_instance.getDevice().getPresentQueue().presentKHR(presentInfo);

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


void Renderer::waitIdle()
{
	m_instance.getDevice().handle.waitIdle();
}
