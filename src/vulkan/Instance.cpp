#include "Instance.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/Utils.h"
#include "vulkan/Window.h"

VulkanInstance::VulkanInstance()
{
}

VulkanInstance::~VulkanInstance()
{
}

void VulkanInstance::create(Window* window)
{
	auto* glfwWindow = window->getGLFWWindow();
	createInstance();
	m_debugHelper.create(handle);
	m_swapchain.createSurface(handle, glfwWindow);
	m_device.create(handle, m_swapchain.getSurface());
	m_swapchain.create(m_device.handle, m_device.getDevice(), glfwWindow);
}

void VulkanInstance::createInstance()
{
	if (!DebugHelper::validationLayersSupported())
	{
		spdlog::critical("could not load validation layers");
		throw std::runtime_error("could not load validation layers");
	}

	auto extensions = vulkan_utils::getRequiredExtensions(DebugHelper::validationLayersEnabled());
	vk::ApplicationInfo appInfo("hello triangle", VK_MAKE_VERSION(1, 0, 0), "no engine", VK_MAKE_VERSION(1, 0, 0), vk::ApiVersion10);

	vk::InstanceCreateInfo info;
	info.setPApplicationInfo(&appInfo);
	info.setEnabledExtensionCount(extensions.size());
	info.setPpEnabledExtensionNames(extensions.data());

	auto debugInfo = DebugHelper::createDebugCreateInfo();
	if (DebugHelper::validationLayersEnabled())
	{
		info.setPEnabledLayerNames(DebugHelper::getValidationLayers());
		info.setPNext(static_cast<void*>(&debugInfo));
	}
	else
	{
		info.setEnabledLayerCount(0);
	}

	handle = vk::createInstance(info);
}

void VulkanInstance::destroy()
{
	m_swapchain.destroy();
	m_device.destroy();

	if (DebugHelper::validationLayersEnabled())
		m_debugHelper.destroy();

	handle.destroySurfaceKHR(m_swapchain.getSurface());
	handle.destroy();
}
