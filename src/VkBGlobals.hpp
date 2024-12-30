
#pragma once
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "Input.hpp"
#define CASCADE_COUNT 4
extern VkDevice device;
extern VkInstance instance;
extern VkPhysicalDevice physicalDevice;
extern VkPhysicalDeviceProperties physicalDeviceProperties;

extern std::vector<const char*> validationLayers;
extern std::vector<const char*> deviceExtensions;
extern GLFWwindow* window;
extern uint32_t graphicsQueueFamily;
extern VkQueue graphicsQueue;
extern VkQueue presentQueue;
extern VkQueue computeQueue;
extern VkCommandPool drawCommandPool;
extern VkCommandPool transientCommandPool; //For short lived command buffers
extern VkCommandPool computeCommandPool; 
extern Input inputInfo;
extern uint32_t framesInFlight;
extern uint32_t windowWidth;
extern uint32_t windowHeight;
