#include "VkBGlobals.hpp"
#include "VkBDrawCommandBuffer.hpp"
#include "VkBVertexBuffer.hpp"
#include <stdexcept>
void
VkBDrawCommandBuffer::createCommandBuffer(VkCommandPool commandPool)
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
VkBDrawCommandBuffer::begin(VkRenderPass renderPass,
			    VkFramebuffer frameBuffer,
			    VkExtent2D extent)
{ 
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
  
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};
  
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();
	
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void
VkBDrawCommandBuffer::end()
{
  vkCmdEndRenderPass(commandBuffer);
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}
void
VkBDrawCommandBuffer::record(VkPipeline pipeline,
			     VkPipelineLayout pipelineLayout,
			     VkBVertexBuffer* vertexBuffer,
			     VkDescriptorSet* descriptorSet,
			     int offset,
			     int count
			     )
{
 
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkBuffer vertexBuffers[] = {vertexBuffer->vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(commandBuffer, vertexBuffer->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  pipelineLayout,
			  0, 1, descriptorSet, 0, nullptr);


  vkCmdDrawIndexed(commandBuffer, count, 1, offset, 0, 0);
}
void
VkBDrawCommandBuffer::record(VkPipeline pipeline,
			     VkPipelineLayout pipelineLayout,
			     VkBVertexBuffer* vertexBuffer,
			     VkDescriptorSet* descriptorSet,
			     Model* model
			     )
{
 
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkBuffer vertexBuffers[] = {vertexBuffer->vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

  vkCmdBindIndexBuffer(commandBuffer, vertexBuffer->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  pipelineLayout,
			  0, 1, descriptorSet, 0, nullptr);


  vkCmdDrawIndexed(commandBuffer, model->indexCount, 1, model->startIndex, model->vertexOffset, 0);
}


