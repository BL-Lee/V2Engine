#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <optional>
#include <vector>
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);


class VkBDeviceSelection
{
public:
  //Physical device -----------------------------------------------
  static VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface,const std::vector<const char*>&deviceExtensions);
  static bool isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,const std::vector<const char*>&deviceExtensions);
  static bool checkDeviceExtensionSupport(VkPhysicalDevice device,const std::vector<const char*>&deviceExtensions);
};
