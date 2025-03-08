#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <vector>
#include "image.hpp"

// Note 1: we don't have multi layered swapchains
// Note 2: each swapchain is relative to a single surface

namespace ignis {

class Device;
class Image;
class Semaphore;
class Fence;
enum class ColorFormat;

class Swapchain {
public:
	struct CreateInfo {
		const Device* device{nullptr};
		VkExtent2D extent{0, 0};
		ColorFormat format{VK_FORMAT_R8G8B8A8_UNORM};
		VkColorSpaceKHR colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		VkSurfaceKHR surface{VK_NULL_HANDLE};
		VkPresentModeKHR presentMode{VK_PRESENT_MODE_FIFO_KHR};
	};

	Swapchain(CreateInfo);
	~Swapchain();

	struct PresentInfo {
		Image* srcImage;
		uint32_t queueIndex;
		std::vector<const Semaphore*>
			waitSemaphores;	 // they are relative to the blitting
		std::vector<const Semaphore*>
			signalSemaphores;  // they are relative to the presenting
	};

	Image& getCurrentImage() { return *m_images[m_currentImageIndex]; }

	Image& acquireNextImage(const Semaphore* signalSemaphore);

	uint32_t getImagesCount() const { return m_images.size(); }

	void present(PresentInfo);

private:
	const Device& m_device;
	VkSwapchainKHR m_swapchain{nullptr};
	VkSurfaceKHR m_surface;
	std::vector<std::unique_ptr<Image>> m_images;
	uint32_t m_currentImageIndex{0};
	VkExtent2D m_extent{0, 0};

	std::unique_ptr<Semaphore> acquiredImageSem;
	std::unique_ptr<Semaphore> blittedImageSem;
	std::unique_ptr<Fence> blitFence;

public:
	Swapchain(const Swapchain&) = delete;
	Swapchain(Swapchain&&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain& operator=(Swapchain&&) = delete;
};

}  // namespace ignis
