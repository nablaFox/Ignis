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
	friend class Device;

public:
	struct CreateInfo {
		VkExtent2D extent{0, 0};
		ColorFormat swapchainFormat{VK_FORMAT_R8G8B8A8_UNORM};
		VkColorSpaceKHR colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		VkPresentModeKHR presentMode{VK_PRESENT_MODE_FIFO_KHR};
		VkSurfaceKHR surface{nullptr};
	};

	Swapchain(VkDevice, VkSwapchainKHR, const CreateInfo&);

	~Swapchain();

	Image& getCurrentImage();

	Image& acquireNextImage(const Semaphore* signalSemaphore = nullptr);

	VkSwapchainKHR getHandle() const { return m_swapchain; }

	uint32_t getImagesCount() const;

	void presentCurrent(const PresentInfo&);

private:
	VkDevice m_device;
	VkSwapchainKHR m_swapchain;

	CreateInfo m_creationInfo;

	std::vector<Image> m_images;
	uint32_t m_currentImageIndex{0};

	static Swapchain allocateSwapchain(VkDevice,
									   VkPhysicalDevice,
									   const CreateInfo&);

public:
	Swapchain(const Swapchain&) = delete;
	Swapchain(Swapchain&&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain& operator=(Swapchain&&) = delete;
};

}  // namespace ignis
