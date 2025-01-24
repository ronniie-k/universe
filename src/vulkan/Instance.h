#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <spdlog/spdlog.h>

#include "vulkan/Device.h"
#include "vulkan/DebugHelper.h"
#include "vulkan/pipeline/GraphicsPipeline.h"
#include "vulkan/swapchain/Swapchain.h"

class Window;

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

	void create(Window* window);
	void destroy();

	VulkanDevice& getDevice() { return m_device; }
	VulkanSwapchain& getSwapchain() { return m_swapchain; }

	vk::Instance handle;

private:
	void createInstance();

private:
	DebugHelper m_debugHelper;
	VulkanDevice m_device;
	VulkanSwapchain m_swapchain;
};