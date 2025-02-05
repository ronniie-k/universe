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

	void create();
	void destroy();

	vk::Instance handle;

private:
	void createInstance();

private:
	DebugHelper m_debugHelper;
};