#include "vkDebug.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>

//Validation and Debug ----------------------------------------------
void DebugUtils::setupDebugMessenger(VkInstance instance) {


  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  populateDebugMessengerCreateInfo(createInfo);


  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("Failed to set up debug messenger");
  }
  
}

void DebugUtils::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = 
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;// |	
  //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
  createInfo.messageType = \
    //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;

    
}

//Have to look up address of vkCreateDebugUtilsMessengerExt ourself
//Since its an extension function. 
VkResult DebugUtils::CreateDebugUtilsMessengerEXT(VkInstance instance,
						  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
						  const VkAllocationCallbacks* pAllocator,
						  VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DebugUtils::DestroyDebugUtilsMessengerEXT(VkInstance instance,
					       VkDebugUtilsMessengerEXT debugMessenger,
					       const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr){
    func(instance, debugMessenger, pAllocator);
  } else {
    throw std::runtime_error("failed to find vkDestroyDebugUtilsMessengerEXT");
  }
}



bool DebugUtils::checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
  std::cout << "CHECKING VALIDATION" << std::endl;
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char* layerName : validationLayers)
    {
      bool layerFound = false;
	
      for (const VkLayerProperties& layerProperties : availableLayers) {
	if (strcmp(layerName, layerProperties.layerName) == 0) {
	  layerFound = true;
	  break;
	}
      }
	
      if (!layerFound)
	return false;

    }
    
  return true;
}



VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtils::debugCallback(
							 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
							 VkDebugUtilsMessageTypeFlagsEXT messageType,
							 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
							 void* pUserData 
							 ) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

