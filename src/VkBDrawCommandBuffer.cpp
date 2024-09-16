#include "VkBDrawCommandBuffer.hpp"
#include "VkBVertexBuffer.hpp"
#include <stdexcept>
void
VkBDrawCommandBuffer::createCommandBuffer(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; //@todo: hints
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
}
void
VkBDrawCommandBuffer::record(VkRenderPass renderPass,
			 VkFramebuffer frameBuffer,
			 VkExtent2D extent,
			 VkPipeline pipeline,
			 VkBVertexBuffer* vertexBuffer)
{
  vkResetCommandBuffer(commandBuffer, 0);//This should be taken out. I dont think we always do this;
  
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
	
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = frameBuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = extent;
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;
	
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkBuffer vertexBuffers[] = {vertexBuffer->buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(commandBuffer, vertexBuffer->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  /*
    Im switching this to static. So its set in createGraphicsPipeline now
    //Since we declared this as dynamic, I think this will basically always stay like this though
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain.extent.width);
    viewport.height = static_cast<float>(swapChain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  */
  //vkCmdDraw(commandBuffer, vertexBuffer->vertexCount, 1, 0, 0);
  vkCmdDrawIndexed(commandBuffer, vertexBuffer->indexCount, 1, 0, 0, 0);
  vkCmdEndRenderPass(commandBuffer);
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}


