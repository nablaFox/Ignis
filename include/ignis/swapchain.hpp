#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

// Note 1: we don't have multi layered swapchains
// Note 2: each swapchain is relative to a single surface

namespace ignis {

class Semaphore;
class Image;
enum class ColorFormat;

struct PresentInfo {
	Image* srcImage;
	VkQueue presentationQueue;
	std::vector<const Semaphore*> waitSemaphores;
};

class Swapchain {
public:
	struct CreateInfo {
		uint32_t width;
		uint32_t height;
		ColorFormat swapchainFormat{VK_FORMAT_R8G8B8A8_UNORM};
		VkColorSpaceKHR colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		VkPresentModeKHR presentMode{VK_PRESENT_MODE_FIFO_KHR};
		VkSurfaceKHR surface{nullptr};
	};

	Swapchain(VkDevice, VkPhysicalDevice, const CreateInfo&);

	~Swapchain();

	Image& getCurrentImage();

	Image& acquireNextImage(const Semaphore* signalSemaphore = nullptr);

	VkSwapchainKHR getHandle() const { return m_swapchain; }

	uint32_t getImagesCount() const;

	void presentCurrent(const PresentInfo&);

private:
	VkDevice m_device;
	CreateInfo m_creationInfo;

	VkSwapchainKHR m_swapchain{nullptr};
	uint32_t m_currentImageIndex{0};
	std::vector<Image> m_images;

public:
	Swapchain(const Swapchain&) = delete;
	Swapchain(Swapchain&&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain& operator=(Swapchain&&) = delete;
};

}  // namespace ignis
