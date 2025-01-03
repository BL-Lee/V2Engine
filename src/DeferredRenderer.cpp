#include "DeferredRenderer.hpp"
void DeferredRenderer::init(VkBSwapChain* swapChain)
{
  fullscreenQuad.create(6 * sizeof(Vertex));
  Vertex vertices[6];
  vertices[0].pos = {-1.0,-1.0,0.0};
  vertices[1].pos = {-1.0,1.0,0.0};
  vertices[2].pos = {1.0,1.0,0.0};
  vertices[3].pos = {1.0,1.0,0.0};
  vertices[4].pos = {1.0,-1.0,0.0};
  vertices[5].pos = {-1.0,-1.0,0.0};
  vertices[0].texCoord = {0.0,0.0};
  vertices[1].texCoord = {0.0,1.0};
  vertices[2].texCoord = {1.0,1.0};
  vertices[3].texCoord = {1.0,1.0};
  vertices[4].texCoord = {1.0,0.0};
  vertices[5].texCoord = {0.0,0.0};
  
  uint32_t s; //throwaway, dont need the offset
  fullscreenQuad.fill(vertices, 6, &s);
  
  mode = 0;
  deferredRenderPass.addColourAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,  false, 0); //normal
  deferredRenderPass.addColourAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,  false, 1); //albedo
  deferredRenderPass.addColourAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,  false, 2); //uv
  deferredRenderPass.addDepthAttachment(3);
  deferredRenderPass.createRenderPass(true);

  albedoTexture.createTextureImage(VKB_TEXTURE_TYPE_RGBA_HDR,
					 swapChain->extent.width, swapChain->extent.height,
					 nullptr);
  normalTexture.createTextureImage(VKB_TEXTURE_TYPE_RGBA_HDR,
					 swapChain->extent.width, swapChain->extent.height,
					 nullptr);
  uvTexture.createTextureImage(VKB_TEXTURE_TYPE_RGBA_HDR,
				   swapChain->extent.width, swapChain->extent.height,
				   nullptr);

  ssaoTexture.createTextureImage(VKB_TEXTURE_TYPE_R_HDR,
				   swapChain->extent.width, swapChain->extent.height,
				   nullptr);

  depthTexture.createTextureImage(VKB_TEXTURE_TYPE_DEPTH,
					 swapChain->extent.width, swapChain->extent.height,
					 nullptr);
  VkImageView attachments[] = { normalTexture.imageView,
				albedoTexture.imageView,
				uvTexture.imageView,
				depthTexture.imageView
  };

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


  diffuseAtlas.init(1024,1024, VKB_TEXTURE_TYPE_SAMPLED_RGBA, 128);
  
  compositeUniformPool.create(1, framesInFlight, 0, true);
  compositeUniformPool.addImage(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //normal
  compositeUniformPool.addImage(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE); //normal
  compositeUniformPool.addImage(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //albedo
  compositeUniformPool.addImage(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE); //albedo
  compositeUniformPool.addImage(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //depth
  compositeUniformPool.addImage(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //uv
  compositeUniformPool.addImage(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //ssao
  compositeUniformPool.addImage(7, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE); //ssao
  compositeUniformPool.addImage(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER); //diffuse atlas


  compositeUniformPool.createDescriptorSetLayout();
  
  VkBTexture* texs[] = { &normalTexture,
			 &normalTexture,
			 &albedoTexture,
			 &albedoTexture,
			 &depthTexture,
			 &uvTexture,
			 &ssaoTexture,
			 &ssaoTexture,
			 &diffuseAtlas.atlas
  };

  compositeUniform.allocateDescriptorSets(&compositeUniformPool, texs, nullptr);

  drawCommandBuffer.createCommandBuffer(drawCommandPool);
  compositeCommandBuffer.createCommandBuffer(drawCommandPool);
}
void DeferredRenderer::destroy()
{
  fullscreenQuad.destroy();
  vkDestroyRenderPass(device, deferredRenderPass.renderPass, nullptr);
  albedoTexture.destroy();
  normalTexture.destroy();
  depthTexture.destroy();
  ssaoTexture.destroy();
  uvTexture.destroy();
  compositeUniformPool.destroy();
  compositeUniform.destroy();
  diffuseAtlas.destroy();
  vkDestroyFramebuffer(device, deferredFramebuffer, nullptr);
  deferredPipeline.destroy();
}

void DeferredRenderer::setCompositeInformation(VkBGraphicsPipeline pipeline,
					       VkBRenderPass renderPass,
					       VkFramebuffer framebuffer )
{
  compositePipeline = pipeline;
  compositeFramebuffer = framebuffer;
  compositeRenderPass = renderPass;
}

void DeferredRenderer::begin( VkExtent2D extent  )
{
  vkResetCommandBuffer(drawCommandBuffer.commandBuffer, 0);
    
  drawCommandBuffer.begin(deferredRenderPass,
			  deferredFramebuffer,
			  extent);
	
  vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, deferredPipeline.pipeline);
  //deferredPipeline = pipeline;
}

void DeferredRenderer::bindDescriptorSet( VkDescriptorSet* dSet, uint32_t ind )
{
  vkCmdBindDescriptorSets(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  deferredPipeline.layout,
			  ind, 1, dSet, 0, nullptr);
}
void DeferredRenderer::bindDescriptorSetComposite( VkDescriptorSet* dSet, uint32_t ind )
{
  vkCmdBindDescriptorSets(compositeCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  compositePipeline.layout,
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



void DeferredRenderer::submitDeferred(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkFence fence)
{

  VkSubmitInfo submitInfo{};
  //  std::vector<VkSemaphore> waitSemaphores = {deferredPassFinishedSemaphore, probeInfoFinishedSemaphore};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //Wait for this stage before writing to image
  submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.size();
  submitInfo.pSignalSemaphores = signalSemaphores.data();
  submitInfo.pCommandBuffers = &drawCommandBuffer.commandBuffer;
  
  drawCommandBuffer.end();

  //Syncronization info for graphics 
  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }
  mode = 1;
}
void DeferredRenderer::beginComposite(VkExtent2D extent, VkFramebuffer framebuffer, VkBGraphicsPipeline pipeline)
{
  compositePipeline = pipeline;
  if (!mode)
    {
      throw std::runtime_error("Attempting to submit composite stage of deferred renderer without submitting deferred stage first");
    }
  
  vkResetCommandBuffer(compositeCommandBuffer.commandBuffer, 0);
  compositeFramebuffer = framebuffer;    
  compositeCommandBuffer.begin(compositeRenderPass,
			  compositeFramebuffer,
			  extent);
  vkCmdBindPipeline(compositeCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipeline.pipeline);


  
}

void DeferredRenderer::recordComposite()
{
    compositeCommandBuffer.record(compositePipeline.pipeline, compositePipeline.layout, &fullscreenQuad, 0, 6);
}

 void DeferredRenderer::submitComposite(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkFence fence)
{

  compositeCommandBuffer.end();

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //Wait for this stage before writing to image
  submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.size();
  submitInfo.pSignalSemaphores = signalSemaphores.data();
  submitInfo.pCommandBuffers = &compositeCommandBuffer.commandBuffer;

  //Syncronization info for graphics 
  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
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
  vkCmdBindPipeline(compositeCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
  compositePipeline = pipeline;
}
