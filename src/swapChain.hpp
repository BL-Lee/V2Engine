#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include "VkBRenderPass.hpp"



//Handles

/*


  Basically anything that has to do with
  storing and detecting what swap chain stuff is available

  Where the swap chain is essentially a queue
  of images waiting to be presented on screen,
  essentially to sync it with the refresh rate

  vKSwapChains,
  vKImages,
  vKExtents,
  vKImageViews,
  vkframebuffers



  VkDeviceMemory is just a sequence of N bytes in memory.

  VkImage object adds to it e.g. information about the format (so you can address by texels, not bytes).

  VkImageView object helps select only part (array or mip) of the VkImage (like stringView, arrayView or whathaveyou does). Also can help to match to some incompatible interface (by type casting format).

  VkFramebuffer binds a VkImageView with an attachment. (literally a collection of attachments)

  VkRenderpass defines which attachment will be drawn into

 */

class VkBSwapChain {
public:
  
  VkSwapchainKHR swapChain;
  std::vector<VkImage> images; //The actual images
  VkFormat imageFormat;
  VkExtent2D extent;
  std::vector<VkImageView> imageViews; //Generic info about the image, like depth/mips etc
  std::vector<VkFramebuffer> framebuffers;

    struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  void createSwapChain(VkSurfaceKHR surface, GLFWwindow* window);
  
  static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

  void createImageViews();
  void createFramebuffers(VkBRenderPass renderPass, VkImageView& depth);
  
};
