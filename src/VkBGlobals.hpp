#pragma once
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"



extern VkDevice device;
extern VkInstance instance;
extern VkPhysicalDevice physicalDevice;

extern std::vector<const char*> validationLayers;
extern std::vector<const char*> deviceExtensions;

extern VkQueue graphicsQueue;
extern VkQueue presentQueue;
extern VkCommandPool drawCommandPool;
extern VkCommandPool transientCommandPool; //For short lived command buffers
