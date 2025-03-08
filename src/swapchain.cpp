#include <cassert>
#include "swapchain.hpp"
#include "command.hpp"
#include "exceptions.hpp"
#include "image.hpp"
#include "semaphore.hpp"

using namespace ignis;

Swapchain::Swapchain(VkDevice device,
					 VkPhysicalDevice physicalDevice,
					 const CreateInfo& info)
	: m_device(device), m_creationInfo(info) {
	assert(device != nullptr && "Invalid device");
	assert(physicalDevice != nullptr && "Invalid physical device");
	assert(info.surface != nullptr && "Invalid surface");
	assert(info.width > 0 && info.height > 0 && "Invalid swapchain extent");

	// 1. Query the surface capabilities.VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceCapabilitiesKHR capabilities{};

	THROW_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
					physicalDevice, info.surface, &capabilities) != VK_SUCCESS,
				"Failed to get surface capabilities");

	// 2. Query available surface formats.
	uint32_t formatCount{0};
	THROW_ERROR(
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, info.surface,
											 &formatCount, nullptr) != VK_SUCCESS,
		"Failed to get surface formats count");

	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	THROW_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, info.surface,
													 &formatCount,
													 formats.data()) != VK_SUCCESS,
				"Failed to get surface formats");

	// 3. Query available present modes.
	uint32_t presentModeCount{0};
	THROW_ERROR(
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, info.surface, &presentModeCount, nullptr) != VK_SUCCESS,
		"Failed to get present modes count");

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	THROW_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(
					physicalDevice, info.surface, &presentModeCount,
					presentModes.data()) != VK_SUCCESS,
				"Failed to get present modes");

	// 4. Choose the present mode - fallback to FIFO
	VkPresentModeKHR presentMode{VK_PRESENT_MODE_FIFO_KHR};

	for (const auto& mode : presentModes) {
		if (mode == info.presentMode) {
			presentMode = mode;
			break;
		}
	}

	// 5. Determine extent
	VkExtent2D swapExtent = capabilities.currentExtent;

	if (capabilities.currentExtent.width == UINT32_MAX) {
		swapExtent = {info.width, info.height};

		swapExtent.width =
			std::max(capabilities.minImageExtent.width,
					 std::min(capabilities.maxImageExtent.width, swapExtent.width));

		swapExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, swapExtent.height));
	}

	// 6. Surface format
	VkSurfaceFormatKHR chosenFormat{};

	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		chosenFormat.format = static_cast<VkFormat>(info.swapchainFormat);
		chosenFormat.colorSpace = info.colorSpace;
	} else {
		bool found = false;
		for (const auto& availableFormat : formats) {
			if (availableFormat.format ==
					static_cast<VkFormat>(info.swapchainFormat) &&
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
	VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = info.surface,
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

	THROW_ERROR(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain) !=
					VK_SUCCESS,
				"Failed to create swapchain");

	// 9. get images
	uint32_t actualImageCount = 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, nullptr);

	std::vector<VkImage> imageHandles(actualImageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount,
							imageHandles.data());

	m_images.reserve(actualImageCount);

	for (const auto& handle : imageHandles) {
		Image::CreateInfo imageCreateInfo{
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					 VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.aspect = VK_IMAGE_ASPECT_COLOR_BIT,
			.width = info.width,
			.height = info.height,
			.format = static_cast<VkFormat>(info.swapchainFormat),
			.optimalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.sampleCount = VK_SAMPLE_COUNT_1_BIT,
		};

		Image image(handle, nullptr, std::move(imageCreateInfo));

		m_images.push_back(std::move(image));
	}
}

Swapchain::~Swapchain() {
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

Image& Swapchain::acquireNextImage(const Semaphore* signalSemaphore) {
	THROW_ERROR(vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
									  signalSemaphore->getHandle(), VK_NULL_HANDLE,
									  &m_currentImageIndex) != VK_SUCCESS,
				"Failed to acquire next image");

	return m_images[m_currentImageIndex];
}

Image& Swapchain::getCurrentImage() {
	return m_images[m_currentImageIndex];
}

uint32_t Swapchain::getImagesCount() const {
	return m_images.size();
}

void Swapchain::presentCurrent(const PresentInfo& info) {
	std::vector<VkSemaphore> waitSemaphores;

	for (const auto& semaphore : info.waitSemaphores) {
		waitSemaphores.push_back(semaphore->getHandle());
	}

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
		.pWaitSemaphores = waitSemaphores.data(),
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &m_currentImageIndex,
	};

	THROW_VULKAN_ERROR(vkQueuePresentKHR(info.presentationQueue, &presentInfo),
					   "Failed to present swapchain image");
}
