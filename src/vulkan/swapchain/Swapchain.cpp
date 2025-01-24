#include "Swapchain.h"
#include "vulkan/Utils.h"
#include "vulkan/image/ImageView.h"
#include <vulkan/vulkan_handles.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void VulkanSwapchain::create(vk::Device device, vk::PhysicalDevice physicalDevice, GLFWwindow* window)
{
	m_device = device;
	m_physicalDevice = physicalDevice;

	createSwapchain(window);
	createImageViews();
}

void VulkanSwapchain::VulkanSwapchain::destroy()
{
	for (auto framebuffer : m_frameBuffers)
		m_device.destroyFramebuffer(framebuffer);
	m_imageViews.clear();
	m_device.destroySwapchainKHR(handle);
	m_instance.destroySurfaceKHR(m_surface);
}

void VulkanSwapchain::createSurface(vk::Instance instance, GLFWwindow* window)
{
	m_instance = instance;
	if (glfwCreateWindowSurface(m_instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface)) != VK_SUCCESS)
		throw std::runtime_error("failed to create vulkan surface!");
}

void VulkanSwapchain::createSwapchain(GLFWwindow* window)
{
	vulkan_utils::SwapchainSupportInfo info = vulkan_utils::getSwapchainSupportInfo(m_physicalDevice, m_surface);

	vk::SurfaceFormatKHR format = chooseSurfaceFormat(info.formats);
	vk::PresentModeKHR presentMode = choosePresentMode(info.presentModes);
	vk::Extent2D extent = chooseExtent(info.capabilities, window);

	uint32_t imgCount = info.capabilities.minImageCount + 1;

	if (info.capabilities.maxImageCount > 0 && imgCount > info.capabilities.maxImageCount)
		imgCount = info.capabilities.maxImageCount;

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setSurface(m_surface);
	createInfo.setMinImageCount(imgCount);
	createInfo.setImageFormat(format.format);
	createInfo.setImageColorSpace(format.colorSpace);
	createInfo.setImageExtent(extent);
	createInfo.setImageArrayLayers(1);
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	vulkan_utils::QueueFamilyIndices indices = vulkan_utils::findQueueFamilies(m_physicalDevice, m_surface);
	std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
		createInfo.setQueueFamilyIndices(queueFamilyIndices);
	}
	else
	{
		createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	}

	createInfo.setPreTransform(info.capabilities.currentTransform);
	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	createInfo.setPresentMode(presentMode);
	createInfo.setClipped(true);
	createInfo.setOldSwapchain(VK_NULL_HANDLE);

	handle = m_device.createSwapchainKHR(createInfo);

	m_images = m_device.getSwapchainImagesKHR(handle);
	m_format = format.format;
	m_extent = extent;
}

void VulkanSwapchain::createImageViews()
{
	for (size_t i = 0; i < m_images.size(); i++)
	{
		VulkanImageView view;
		view.create(m_device, m_images[i], m_format);
		m_imageViews.emplace_back(std::move(view));
	}
}

void VulkanSwapchain::createFramebuffers(vk::RenderPass renderPass)
{
	m_renderPass = renderPass;
	for (int i = 0; i < m_images.size(); i++)
	{
		std::array<vk::ImageView, 1> attachments = { m_imageViews[i].handle };
		vk::FramebufferCreateInfo createInfo;
		createInfo.setRenderPass(renderPass);
		createInfo.setAttachments(attachments);
		createInfo.setWidth(m_extent.width);
		createInfo.setHeight(m_extent.height);
		createInfo.setLayers(1);

		auto fb = m_device.createFramebuffer(createInfo);
		m_frameBuffers.emplace_back(fb);
	}
}

vk::SurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> formats)
{
	for (const auto& format : formats)
	{
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return format;
	}
	return formats[0];
}

vk::PresentModeKHR VulkanSwapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
{
	for (const auto& mode : presentModes)
	{
		if (mode == vk::PresentModeKHR::eMailbox)
			return mode;
	}
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapchain::chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VulkanSwapchain::VulkanSwapchain::VulkanSwapchain::recreate(GLFWwindow* window)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	m_device.waitIdle();

	// destroy swapchain components (not the renderpass or surface)
	for (auto framebuffer : m_frameBuffers)
		m_device.destroyFramebuffer(framebuffer);
	m_frameBuffers.clear();
	m_imageViews.clear();
	m_device.destroySwapchainKHR(handle);

	createSwapchain(window);
	createImageViews();
	createFramebuffers(m_renderPass);
}
