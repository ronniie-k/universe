#include "ImageView.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>


void VulkanImageView::create(vk::Device device, vk::Image image, vk::Format format)
{
	m_device = device;
	vk::ImageViewCreateInfo createInfo;
	createInfo.setImage(image);
	createInfo.setViewType(vk::ImageViewType::e2D);
	createInfo.setFormat(format);
	createInfo.components.r = vk::ComponentSwizzle::eIdentity;
	createInfo.components.g = vk::ComponentSwizzle::eIdentity;
	createInfo.components.b = vk::ComponentSwizzle::eIdentity;
	createInfo.components.a = vk::ComponentSwizzle::eIdentity;
	createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	vk::ImageSubresourceRange subresourceRange;

	handle = m_device.createImageView(createInfo);
}

VulkanImageView::~VulkanImageView()
{
	if (handle != VK_NULL_HANDLE)
		m_device.destroyImageView(handle);
}

VulkanImageView& VulkanImageView::operator=(VulkanImageView&& rhs)
{
	if (&rhs != this)
	{
		handle = rhs.handle;
		rhs.handle = VK_NULL_HANDLE;
		m_device = rhs.m_device;
		rhs.m_device = nullptr;
	}
	return *this;
}

VulkanImageView::VulkanImageView(VulkanImageView&& other)
{
	handle = other.handle;
	other.handle = VK_NULL_HANDLE;
	m_device = other.m_device;
	other.m_device = nullptr;
}
