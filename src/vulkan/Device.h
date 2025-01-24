#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>

class VulkanInstance;

class VulkanDevice
{
public:
	~VulkanDevice();

	void create(vk::Instance instance, vk::SurfaceKHR surface);
	void destroy();

	vk::PhysicalDevice getDevice() { return m_physicalDevice; }

	const vk::Queue& getGraphicsQueue() const { return m_graphicsQueue; }
	const vk::Queue& getPresentQueue() const { return m_presentQueue; }

	vk::Device handle;

private:
	void pickPhysicalDevice(vk::SurfaceKHR surface);
	void createDevice(vk::SurfaceKHR surface);

	bool extensionsSupported(vk::PhysicalDevice device);
	bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);

private:
	vk::Instance m_instance;
	vk::PhysicalDevice m_physicalDevice;

	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;

	const std::vector<const char*> m_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};