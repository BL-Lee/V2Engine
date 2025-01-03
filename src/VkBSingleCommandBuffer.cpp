#include "VkBSingleCommandBuffer.hpp"
#include <stdexcept>
VkCommandBuffer vKBeginSingleTimeCommandBuffer()
{
  VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = transientCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}
void vKEndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VkResult result;
  result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to submit queue single time command buffer");

  result = vkQueueWaitIdle(graphicsQueue);
  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to wait for idle queue single time command buffer");

  vkFreeCommandBuffers(device, transientCommandPool, 1, &commandBuffer);
}
/*

  requires some semaphores
void vKEndSingleTimeCommandBufferNonBlocking(VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  

  VkResult result;
  result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  if (result != VK_SUCCESS)
    throw std::runtime_error("Failed to submit queue single time command buffer");

  vkFreeCommandBuffers(device, transientCommandPool, 1, &commandBuffer);
}
*/
