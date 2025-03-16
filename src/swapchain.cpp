#include "swapchain.hpp"
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
Swapchain::Swapchain(const SwapchainCreateInfo& info)
	: m_device(*info.device), m_surface(info.surface) {
	assert(info.extent.width > 0 && info.extent.height > 0 &&
		   "Invalid swapchain extent");

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
	VkSurfaceFormatKHR chosenFormat{};
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		chosenFormat.format = static_cast<VkFormat>(info.format);
		chosenFormat.colorSpace = info.colorSpace;
	} else {
		bool found = false;
		for (const auto& availableFormat : formats) {
			if (availableFormat.format == static_cast<VkFormat>(info.format) &&
				availableFormat.colorSpace == info.colorSpace) {
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
	VkSwapchainCreateInfoKHR const createInfo{
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

	std::vector<VkImage> imageHandles(actualImageCount);
	vkGetSwapchainImagesKHR(m_device.getDevice(), m_swapchain, &actualImageCount,
							imageHandles.data());

	m_images.reserve(actualImageCount);

	for (const auto& handle : imageHandles) {
		ImageCreateInfo info{
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					 VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.width = swapExtent.width,
			.height = swapExtent.height,
			.format = chosenFormat.format,
			.optimalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};

		m_images.push_back(std::make_unique<Image>(m_device, handle, nullptr, info));
	}
}

Swapchain::~Swapchain() {
	vkDestroySwapchainKHR(m_device.getDevice(), m_swapchain, nullptr);
	vkDestroySurfaceKHR(m_device.getInstance(), m_surface, nullptr);
}

Image& Swapchain::acquireNextImage(const Semaphore* signalSemaphore) {
	THROW_ERROR(vkAcquireNextImageKHR(m_device.getDevice(), m_swapchain, UINT64_MAX,
									  signalSemaphore->getHandle(), VK_NULL_HANDLE,
									  &m_currentImageIndex) != VK_SUCCESS,
				"Failed to acquire next image");

	return *m_images[m_currentImageIndex];
}

void Swapchain::presentCurrent(const PresentInfo& info) {
	VkQueue presentationQueue =
		info.presentationQueue ? info.presentationQueue : m_device.getQueue(0);

	std::vector<VkSemaphore> waitSemaphores;

	for (const auto& semaphore : info.waitSemaphores) {
		waitSemaphores.push_back(semaphore->getHandle());
	}

	VkPresentInfoKHR const presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
		.pWaitSemaphores = waitSemaphores.data(),
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &m_currentImageIndex,
	};

	THROW_VULKAN_ERROR(vkQueuePresentKHR(presentationQueue, &presentInfo),
					   "Failed to present swapchain image");
}
