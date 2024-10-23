#include "Device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

Device::Device()
{
}


Device::~Device()
{
	m_instance.destroy();
}

void Device::createInstance()
{
	if (m_enableValidationLayers && !hasValidationLayerSupport())
	{
		spdlog::critical("could not load validation layers");
		throw std::runtime_error("could not load validation layers");
	}
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vk::ApplicationInfo appInfo("hello triangle", VK_MAKE_VERSION(1, 0, 0), "no engine", VK_MAKE_VERSION(1, 0, 0),
								vk::ApiVersion10);

	vk::InstanceCreateInfo info;
	info.setPApplicationInfo(&appInfo);
	info.setEnabledExtensionCount(glfwExtensionCount);
	info.setPpEnabledExtensionNames(glfwExtensions);
	if (m_enableValidationLayers)
	{
		info.setPEnabledLayerNames(m_validationLayers);
	}
	info.setEnabledLayerCount(0);
	m_instance = vk::createInstance(info);
}

void Device::printAvailableExtensions()
{
	std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

	spdlog::info("--available extensions--");
	for (const auto& extension : extensions)
	{
		spdlog::info("{}", extension.extensionName.data());
	}
}

bool Device::hasValidationLayerSupport()
{
	std::vector<vk::LayerProperties> layers = vk::enumerateInstanceLayerProperties();

	for (const char* validationLayerName : m_validationLayers)
	{
		for (const vk::LayerProperties& layer : layers)
		{
			if (strcmp(layer.layerName, validationLayerName) == 0)
			{
				return true;
			}
		}
	}
	return false;
}
