#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <vector>
#include "image.hpp"

// Note 1: we don't have multi layered swapchains
// Note 2: each swapchain is relative to a single surface

namespace ignis {

class Semaphore;
enum class ColorFormat;

struct SwapchainCreateInfo {
	VkExtent2D extent{0, 0};
	ColorFormat format{VK_FORMAT_R8G8B8A8_UNORM};
	VkColorSpaceKHR colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	VkSurfaceKHR surface{VK_NULL_HANDLE};
	VkPresentModeKHR presentMode{VK_PRESENT_MODE_FIFO_KHR};
};

struct PresentInfo {
	VkQueue presentationQueue{nullptr};
	std::vector<const Semaphore*> waitSemaphores;
};

class Swapchain {
public:
	Swapchain(const VkDevice, const VkPhysicalDevice, const SwapchainCreateInfo&);

	~Swapchain();

	Image& getCurrentImage() { return *m_images[m_currentImageIndex]; }

	Image& acquireNextImage(const Semaphore* signalSemaphore);

	uint32_t getImagesCount() const { return m_images.size(); }

	void presentCurrent(const PresentInfo&) const;

private:
	const VkDevice m_device;
	VkSwapchainKHR m_swapchain{nullptr};
	VkSurfaceKHR m_surface;
	std::vector<std::unique_ptr<Image>> m_images;
	uint32_t m_currentImageIndex{0};
	VkExtent2D m_extent{0, 0};

public:
	Swapchain(const Swapchain&) = delete;
	Swapchain(Swapchain&&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain& operator=(Swapchain&&) = delete;
};

}  // namespace ignis
