#pragma once



//VkCommandPool is so thin, feels silly to add a class wrapper

void createCommandPool(VkCommandPool* commandPool,
		       VkDevice device,
		       VkPhysicalDevice physicalDevice,
		       VkSurfaceKHR surface,
		       VkCommandPoolCreateFlags flags
		       ) { /* todo: add hints and type of command pool */
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; ///hints onhow it used
    poolInfo.flags = flags;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    if (vkCreateCommandPool(device, &poolInfo, nullptr, commandPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
    }
}
