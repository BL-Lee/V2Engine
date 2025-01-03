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
VkBDrawCommandBuffer::begin(VkBRenderPass& renderPass,
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
  renderPassInfo.renderPass = renderPass.renderPass;
  renderPassInfo.framebuffer = frameBuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = extent;

  std::vector<VkClearValue> clearValues(renderPass.attachments.size());
  for (int i = 0; i < renderPass.attachments.size(); i++)
    {
      if (renderPass.attachments[i].finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
	  renderPass.attachments[i].finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
	{
	  clearValues[i].depthStencil = {1.0f, 0};
	}
      else
	{
	  clearValues[i].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	}
    }
  /*
  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.2f, 0.2f, 0.2f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};
  */
  renderPassInfo.clearValueCount = (uint32_t)renderPass.attachments.size();
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
VkBDrawCommandBuffer::record(
			     VkPipeline pipeline,
			     VkPipelineLayout pipelineLayout,
			     VkBVertexBuffer* vertexBuffer,
			     int offset,
			     int count
			     )
{
 


  VkBuffer vertexBuffers[] = {vertexBuffer->vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  if (vertexBuffer->indexed)
    vkCmdBindIndexBuffer(commandBuffer, vertexBuffer->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  
  if (vertexBuffer->indexed)
    vkCmdDrawIndexed(commandBuffer, count, 1, offset, 0, 0);
  else
    vkCmdDraw(commandBuffer, count, 1, offset, 0);


}
void
VkBDrawCommandBuffer::record(VkPipeline pipeline,
			     VkPipelineLayout pipelineLayout,
			     VkBVertexBuffer* vertexBuffer,
			     Model* model
			     )
{
 

  VkBuffer vertexBuffers[] = {vertexBuffer->vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  if (vertexBuffer->indexed)
    vkCmdBindIndexBuffer(commandBuffer, vertexBuffer->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdPushConstants(commandBuffer,
		     pipelineLayout,
		     VK_SHADER_STAGE_VERTEX_BIT,
		     24, sizeof(uint32_t),
		     &model->indexIntoModelMatrixBuffer);
  
  if (vertexBuffer->indexed)
    vkCmdDrawIndexed(commandBuffer, model->indexCount, 1, model->startIndex, model->vertexOffset, 0);
  else
    vkCmdDraw(commandBuffer, model->vertexCount, 1, model->vertexOffset, 0);
}


