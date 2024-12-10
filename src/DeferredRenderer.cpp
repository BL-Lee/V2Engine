#include "DeferredRenderer.hpp"
void DeferredRenderer::init(VkBSwapChain* swapChain)
{
  fullscreenQuad.create(6 * sizeof(Vertex));
  Vertex vertices[6];
  vertices[0].pos = {0.0,0.0,0.0};
  vertices[1].pos = {0.0,1.0,0.0};
  vertices[2].pos = {1.0,1.0,0.0};
  vertices[3].pos = {1.0,1.0,0.0};
  vertices[4].pos = {1.0,0.0,0.0};
  vertices[5].pos = {0.0,0.0,0.0};
  vertices[0].texCoord = {0.0,0.0};
  vertices[1].texCoord = {0.0,1.0};
  vertices[2].texCoord = {1.0,1.0};
  vertices[3].texCoord = {1.0,1.0};
  vertices[4].texCoord = {1.0,0.0};
  vertices[5].texCoord = {0.0,0.0};
  uint32_t s;
  fullscreenQuad.fill(vertices, 6, &s);
  mode = 0;
  deferredRenderPass.addColourAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,  false, 0); //normal
  deferredRenderPass.addColourAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, false, 1); //world pos
  deferredRenderPass.addColourAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, false, 2); //UV
  deferredRenderPass.addDepthAttachment(3);
  deferredRenderPass.createRenderPass();

  deferredTextures = (VkBTexture*)malloc(sizeof(VkBTexture) * 4);
  deferredTextures[0].createTextureImage(VKB_TEXTURE_TYPE_RGBA_HDR,
					 swapChain->extent.width, swapChain->extent.height,
					 nullptr);
  deferredTextures[1].createTextureImage(VKB_TEXTURE_TYPE_RGBA_HDR,
					 swapChain->extent.width, swapChain->extent.height,
					 nullptr);
  deferredTextures[2].createTextureImage(VKB_TEXTURE_TYPE_RGBA_HDR,
					 swapChain->extent.width, swapChain->extent.height,
					 nullptr);
  deferredTextures[3].createTextureImage(VKB_TEXTURE_TYPE_DEPTH,
					 swapChain->extent.width, swapChain->extent.height,
					 nullptr);
  VkImageView attachments[] = { deferredTextures[0].imageView,
				deferredTextures[1].imageView,
				deferredTextures[2].imageView,
				deferredTextures[3].imageView  };

  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = deferredRenderPass.renderPass;
  framebufferInfo.attachmentCount = 4;
  framebufferInfo.pAttachments = attachments;
  framebufferInfo.width = swapChain->extent.width;
  framebufferInfo.height = swapChain->extent.height;
  framebufferInfo.layers = 1;

  if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &deferredFramebuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create framebuffer!");
  }

  
  compositeUniformPool.create(1, 1, 0, true);
  compositeUniformPool.addImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  compositeUniformPool.addImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  compositeUniformPool.addImage(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  compositeUniformPool.addImage(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  compositeUniformPool.createDescriptorSetLayout();
  
  VkSampler samplers[] = { deferredTextures[0].textureSampler,
				deferredTextures[1].textureSampler,
				deferredTextures[2].textureSampler,
				deferredTextures[3].textureSampler
  };

  compositeUniform.allocateDescriptorSets(&compositeUniformPool, attachments, samplers, nullptr);




}
void DeferredRenderer::destroy()
{
  fullscreenQuad.destroy();
  vkDestroyRenderPass(device, deferredRenderPass.renderPass, nullptr);
  for (int i = 0; i < 4; i++)
    {
      deferredTextures[i].destroy();
    }
  compositeUniformPool.destroy();
  compositeUniform.destroy();
  free(deferredTextures);
  vkDestroyFramebuffer(device, deferredFramebuffer, nullptr);
  //  deferredPipeline.destroy();
}

void DeferredRenderer::setCompositeInformation(VkBGraphicsPipeline pipeline,
					       VkBRenderPass renderPass,
					       VkFramebuffer framebuffer )
{
  compositePipeline = pipeline;
  compositeFramebuffer = framebuffer;
  compositeRenderPass = renderPass;
}

void DeferredRenderer::begin( VkExtent2D extent, VkFramebuffer framebuffer,
			     VkBGraphicsPipeline pipeline
			     )
{
  vkResetCommandBuffer(drawCommandBuffer.commandBuffer, 0);
    
  drawCommandBuffer.begin(deferredRenderPass.renderPass,
			  framebuffer,
			  extent);
	
  vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
  deferredPipeline = pipeline;
}

void DeferredRenderer::bindDescriptorSet( VkDescriptorSet* dSet, uint32_t ind )
{
  vkCmdBindDescriptorSets(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  deferredPipeline.layout,
			  ind, 1, dSet, 0, nullptr);
}

void DeferredRenderer::record(VkBVertexBuffer* vertexBuffer, Model* model)
{
  drawCommandBuffer.record(deferredPipeline.pipeline,
			   deferredPipeline.layout,
			   vertexBuffer,
			   model);
}

void DeferredRenderer::record(VkBVertexBuffer* vertexBuffer, uint32_t start, uint32_t stop)
{
  drawCommandBuffer.record(deferredPipeline.pipeline,
			   deferredPipeline.layout,
			   vertexBuffer,
			   start, stop);
}

void DeferredRenderer::submitDeferred(  VkSubmitInfo* submitInfo, VkFence fence)
{
  drawCommandBuffer.end();
  submitInfo->pCommandBuffers = &drawCommandBuffer.commandBuffer;
  //Syncronization info for graphics 
  if (vkQueueSubmit(graphicsQueue, 1, submitInfo, fence) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }
  mode = 1;
}
void DeferredRenderer::submitComposite(VkSubmitInfo* submitInfo, VkFence fence, VkExtent2D extent)
{
  if (!mode)
    {
      throw std::runtime_error("Attempting to submit composite stage of deferred renderer without submitting deferred stage first");
    }
  
  vkResetCommandBuffer(drawCommandBuffer.commandBuffer, 0);
    
  drawCommandBuffer.begin(compositeRenderPass.renderPass,
			  compositeFramebuffer,
			  extent);
	
  vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipeline.pipeline);
  drawCommandBuffer.record(compositePipeline.pipeline, compositePipeline.layout, &fullscreenQuad, 0, 6);
  drawCommandBuffer.end();
  submitInfo->pCommandBuffers = &drawCommandBuffer.commandBuffer;
  //Syncronization info for graphics 
  if (vkQueueSubmit(graphicsQueue, 1, submitInfo, fence) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }
  mode = 0;
}

void DeferredRenderer::changeDeferredPipeline(VkBGraphicsPipeline pipeline)
{
  vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
  deferredPipeline = pipeline;
}
void DeferredRenderer::changeCompositePipeline(VkBGraphicsPipeline pipeline)
{
  vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
  compositePipeline = pipeline;
}
