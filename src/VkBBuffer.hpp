#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"


uint32_t findMemoryType(uint32_t typeFilter,
			VkMemoryPropertyFlags properties);

void createBuffer(VkDeviceSize size,
		  VkBufferUsageFlags usage,
		  VkMemoryPropertyFlags properties,
		  VkBuffer& buffer,
		  VkDeviceMemory& bufferMemory);

//Command pool should be transientCommandPool
void copyBuffer(VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		uint32_t srcOffset,
		uint32_t dstOffset
		);
