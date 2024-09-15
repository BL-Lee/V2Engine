#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>
class DebugUtils {
public:
  VkDebugUtilsMessengerEXT debugMessenger;

  //Validation and Debug ----------------------------------------------
  void setupDebugMessenger(VkInstance instance);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
  //Have to look up address of vkCreateDebugUtilsMessengerExt ourself
  //Since its an extension function. 
  VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
					const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
					const VkAllocationCallbacks* pAllocator,
					VkDebugUtilsMessengerEXT* pDebugMessenger) ;

  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
				     VkDebugUtilsMessengerEXT debugMessenger,
				     const VkAllocationCallbacks* pAllocator) ;

  bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
						      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
						      VkDebugUtilsMessageTypeFlagsEXT messageType,
						      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
						      void* pUserData 
						      );

};

