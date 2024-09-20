#include "swapChain.hpp"
#include "DeviceSelection.hpp"
#include <stdexcept>
#include <algorithm>
#include <limits>
#include "VkBGlobals.hpp"
#include "VkBTexture.hpp"
void VkBSwapChain::createSwapChain(VkSurfaceKHR surface, GLFWwindow* window) {
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  extent = chooseSwapExtent(swapChainSupport.capabilities, window);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //Images can be used across queues without explicit ownership transfers
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  }
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;


  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create swap chain");
  }
    
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
  images.resize(imageCount);

  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());
  imageFormat = surfaceFormat.format;
    
}

VkBSwapChain::SwapChainSupportDetails VkBSwapChain::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
  VkBSwapChain::SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());     
  }

    
    
  return details;
}

VkSurfaceFormatKHR VkBSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

  //Should switch to float later
    
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
	availableFormat.colorSpace ==
	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  return availableFormats[0];
    
}
    
VkPresentModeKHR VkBSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
  //return VK_PRESENT_MODE_IMMEDIATE_KHR;
  //return VK_PRESENT_MODE_MAILBOX_KHR; // triple buffering
  return VK_PRESENT_MODE_FIFO_KHR; //double buffering basically (why jittery?)
  //Look back in the tutorial if you want mailbox (triple buffering) or immediate mode (single buffering)
}

VkExtent2D VkBSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
      
    VkExtent2D actualExtent = {
      (uint32_t)width,
      (uint32_t)height,
    };

    actualExtent.width = std::clamp(actualExtent.width,
				    capabilities.minImageExtent.width,
				    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height,
				     capabilities.minImageExtent.height,
				     capabilities.maxImageExtent.height);

    return actualExtent;
  }
    
}

void VkBSwapChain::createImageViews() {
  imageViews.resize(images.size());

  for (int i = 0; i < images.size(); i++) {
    imageViews[i] = VkBTexture::createImageView(images[i], imageFormat);
  }
}
  
void VkBSwapChain::createFramebuffers(VkBRenderPass renderPass) {
  framebuffers.resize(imageViews.size());
  for (size_t i = 0; i < imageViews.size(); i++) {
    VkImageView attachments[] = { imageViews[i] };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

	

