#pragma once

#include <vulkan/vulkan.hpp>

struct VulkanImageView
{
	VulkanImageView() = default;
	~VulkanImageView();

	VulkanImageView(const VulkanImageView& other) = delete;
	VulkanImageView& operator=(const VulkanImageView& rhs) = delete;

	VulkanImageView(VulkanImageView&& other);
	VulkanImageView& operator=(VulkanImageView&& rhs);

	void create(vk::Device device, vk::Image image, vk::Format format);

	vk::ImageView handle;

private:
	vk::Device m_device;
};