#include "DeviceSelection.hpp"
#include "swapChain.hpp"
#include <stdexcept>
#include <set>
#include <cstring>
#include <optional>


QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  for (int i = 0; i < queueFamilyCount; i++)
    {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	indices.graphicsFamily = i;
      }
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (presentSupport) {
	indices.presentFamily = i;
      }
      if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
	indices.computeFamily = i;
      }

      if (indices.isComplete()) {
	break;
      }
    }
    
  return indices;
}

//Physical device -----------------------------------------------
VkPhysicalDevice VkBDeviceSelection::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>&deviceExtensions) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("No GPUs detected with Vulkan support");
  }
    
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  VkPhysicalDevice selectedDevice;
    
  for (const VkPhysicalDevice& device : devices) {
    if (isPhysicalDeviceSuitable(device, surface, deviceExtensions)) {
      selectedDevice = device;
      break;
    }
  }

  if (selectedDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("No GPUs detected with suitable Vulkan support");      
  }
    
  return selectedDevice;
}


  
bool VkBDeviceSelection::isPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>&deviceExtensions) {
  QueueFamilyIndices indices = findQueueFamilies(device, surface);
  bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    VkBSwapChain::SwapChainSupportDetails swapChainSupport = VkBSwapChain::querySwapChainSupport(device, surface);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    
  return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
  //return deviceFeatures.geometryShader; Can check for other things too
}

  
bool VkBDeviceSelection::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>&deviceExtensions) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);

  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const VkExtensionProperties& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

