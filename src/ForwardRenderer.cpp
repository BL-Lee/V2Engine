#include "ForwardRenderer.hpp"
void ForwardRenderer::begin( VkExtent2D extent, VkFramebuffer framebuffer,
			     VkBGraphicsPipeline pipeline
			     )
{
  vkResetCommandBuffer(drawCommandBuffer.commandBuffer, 0);
    
  drawCommandBuffer.begin(renderPass,
			  framebuffer,
			  extent);
	
  vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
  currentPipeline = pipeline;
}

void ForwardRenderer::bindDescriptorSet( VkDescriptorSet* dSet, uint32_t ind )
{
  vkCmdBindDescriptorSets(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  currentPipeline.layout,
			  ind, 1, dSet, 0, nullptr);
}

void ForwardRenderer::record(VkBVertexBuffer* vertexBuffer, Model* model)
{
  drawCommandBuffer.record(currentPipeline.pipeline,
			   currentPipeline.layout,
			   vertexBuffer,
			   model);
}

void ForwardRenderer::record(VkBVertexBuffer* vertexBuffer, uint32_t start, uint32_t stop)
{
  drawCommandBuffer.record(currentPipeline.pipeline,
			   currentPipeline.layout,
			   vertexBuffer,
			   start, stop);
}

void ForwardRenderer::submit(  VkSubmitInfo* submitInfo, VkFence fence)
{
  drawCommandBuffer.end();
  submitInfo->pCommandBuffers = &drawCommandBuffer.commandBuffer;
  //Syncronization info for graphics 
  if (vkQueueSubmit(graphicsQueue, 1, submitInfo, fence) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

}

void ForwardRenderer::changePipeline(VkBGraphicsPipeline pipeline)
{
  vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
  currentPipeline = pipeline;
}

