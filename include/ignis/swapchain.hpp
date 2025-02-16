#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <vector>
#include "image_data.hpp"

// Note 1: we don't have multi layered swapchains
// Note 2: each swapchain is relative to a single surface

namespace ignis {

class Device;
class Image;
class Semaphore;
class Fence;
struct ImageData;

class Swapchain {
public:
	struct CreateInfo {
		const Device* device;
		VkExtent2D extent;
		VkSurfaceKHR surface;
		VkPresentModeKHR presentMode;
	};

	Swapchain(CreateInfo);
	~Swapchain();

	struct PresentInfo {
		Image* image;
		uint32_t queueIndex;
		std::vector<const Semaphore*>
			waitSemaphores;	 // they are relative to the blitting
		std::vector<const Semaphore*>
			signalSemaphores;  // they are relative to the presenting
	};

	ImageData& getCurrentImage() { return m_images[m_currentImageIndex]; }

	ImageData& acquireNextImage(const Semaphore* signalSemaphore);

	uint32_t getImagesCount() const { return m_images.size(); }

	void present(PresentInfo);

private:
	const Device& m_device;
	VkSwapchainKHR m_swapchain{nullptr};
	VkSurfaceKHR m_surface;
	std::vector<ImageData> m_images;
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
