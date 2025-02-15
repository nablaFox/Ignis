#include "swapchain.hpp"
#include <vulkan/vulkan_core.h>
#include <cassert>
#include "command.hpp"
#include "device.hpp"
#include "exceptions.hpp"
#include "image.hpp"
#include "semaphore.hpp"
#include "fence.hpp"

using namespace ignis;

// the user should provide the correct instance & device extensions
// when creating the device so here we don't do any check
// (if it throws, it's the user's fault)
Swapchain::Swapchain(CreateInfo info) : m_device(*info.device) {
	VkPhysicalDevice physicalDevice = m_device.getPhysicalDevice();
	VkSurfaceKHR surface = info.surface;

	// 1. Query the surface capabilities.VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceCapabilitiesKHR capabilities;

	THROW_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
					physicalDevice, surface, &capabilities) != VK_SUCCESS,
				"Failed to get surface capabilities");

	// 2. Query available surface formats.
	uint32_t formatCount = 0;
	THROW_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(
					physicalDevice, surface, &formatCount, nullptr) != VK_SUCCESS,
				"Failed to get surface formats count");

	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	THROW_ERROR(
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
											 formats.data()) != VK_SUCCESS,
		"Failed to get surface formats");

	// 3. Query available present modes.
	uint32_t presentModeCount = 0;
	THROW_ERROR(
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, surface, &presentModeCount, nullptr) != VK_SUCCESS,
		"Failed to get present modes count");

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	THROW_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(
					physicalDevice, surface, &presentModeCount,
					presentModes.data()) != VK_SUCCESS,
				"Failed to get present modes");

	// 4. Choose the present mode - fallback to FIFO
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& mode : presentModes) {
		if (mode == info.presentMode) {
			presentMode = mode;
			break;
		}
	}

	// 5. Determine extent
	VkExtent2D swapExtent = capabilities.currentExtent;

	if (capabilities.currentExtent.width == UINT32_MAX) {
		swapExtent = info.extent;

		swapExtent.width =
			std::max(capabilities.minImageExtent.width,
					 std::min(capabilities.maxImageExtent.width, swapExtent.width));

		swapExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, swapExtent.height));

		m_extent = swapExtent;
	}

	// 6. Surface format
	VkSurfaceFormatKHR chosenFormat;
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		chosenFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		chosenFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	} else {
		bool found = false;
		for (const auto& availableFormat : formats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				chosenFormat = availableFormat;
				found = true;
				break;
			}
		}

		if (!found) {
			chosenFormat = formats[0];
		}
	}

	// 7. Image count
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	// 8. Create the swapchain
	VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = chosenFormat.format,
		.imageColorSpace = chosenFormat.colorSpace,
		.imageExtent = swapExtent,
		.imageArrayLayers = 1,
		.imageUsage =
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	THROW_ERROR(vkCreateSwapchainKHR(m_device.getDevice(), &createInfo, nullptr,
									 &m_swapchain) != VK_SUCCESS,
				"Failed to create swapchain");

	// 9. Get the swapchain images
	uint32_t actualImageCount = 0;
	vkGetSwapchainImagesKHR(m_device.getDevice(), m_swapchain, &actualImageCount,
							nullptr);

	m_images.resize(actualImageCount);
	vkGetSwapchainImagesKHR(m_device.getDevice(), m_swapchain, &actualImageCount,
							m_images.data());

	acquiredImageSem = std::make_unique<Semaphore>(m_device);
	blittedImageSem = std::make_unique<Semaphore>(m_device);
	blitFence = std::make_unique<Fence>(m_device);

	// transition all the images to transfer dst
	Command cmd(m_device, 0);

	cmd.begin();

	for (auto& image : m_images) {
		cmd.transitionImageLayout(image, VK_IMAGE_ASPECT_COLOR_BIT,
								  VK_IMAGE_LAYOUT_UNDEFINED,
								  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	cmd.end();

	Fence fence(m_device);

	m_device.submitCommands({{&cmd}}, fence);

	fence.wait();
}

Swapchain::~Swapchain() {
	vkDestroySwapchainKHR(m_device.getDevice(), m_swapchain, nullptr);
}

void Swapchain::acquireNextImage(const Semaphore* signalSemaphore) {
	THROW_ERROR(vkAcquireNextImageKHR(m_device.getDevice(), m_swapchain, UINT64_MAX,
									  signalSemaphore->getHandle(), VK_NULL_HANDLE,
									  &m_currentImageIndex) != VK_SUCCESS,
				"Failed to acquire next image");
}

void Swapchain::present(PresentInfo info) {
	assert(info.image->getCurrentLayout() ==
			   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
		   "Image is not in the correct layout");

	blitFence->wait();
	blitFence->reset();

	acquireNextImage(acquiredImageSem.get());

	// PONDER is okay to create the command every time we call this function?
	Command blitCmd(m_device, info.queueIndex);

	blitCmd.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	blitCmd.copyImage(info.image->getImage(), m_images[m_currentImageIndex],
					  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT,
					  VK_IMAGE_ASPECT_COLOR_BIT, info.image->getExtent(), m_extent);

	blitCmd.end();

	info.waitSemaphores.push_back(acquiredImageSem.get());

	SubmitCmdInfo copyCmdInfo{
		.command = &blitCmd,
		.waitSemaphores = std::move(info.waitSemaphores),
		.signalSemaphore = {blittedImageSem.get()},
	};

	m_device.submitCommands({std::move(copyCmdInfo)}, *blitFence);

	// present image
	VkSemaphore waitSem = blittedImageSem.get()->getHandle();

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &waitSem,
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &m_currentImageIndex,
	};

	VkQueue presentQueue = VK_NULL_HANDLE;

	// TODO use exceptions instead of returning a bool in m_device.getQueue
	THROW_ERROR(m_device.getQueue(info.queueIndex, &presentQueue),
				"Failed to fetch present queue");

	THROW_VULKAN_ERROR(vkQueuePresentKHR(presentQueue, &presentInfo),
					   "Failed to present swapchain image");
}
