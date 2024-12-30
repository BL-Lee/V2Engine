#include "SSAOPass.hpp"
#include <vector>
#include "glm/glm.hpp"
#include <stdexcept>
void SSAOPass::init(VkDescriptorSetLayout SSAOLayout,
		    VkDescriptorSetLayout cameraLayout)
{

  VkBTexture* texs[8];
  for (int i = 0; i < 4; i++)
    {
      //TODO dynamic size
      depthSlices[i].createTextureImage(VKB_TEXTURE_TYPE_R_HDR,
					windowWidth / 2,
					windowHeight / 2,
					nullptr);
      //TODO dynamic size
      //TODO lower precision
      ssaoSlices[i].createTextureImage(VKB_TEXTURE_TYPE_R_HDR,
					windowWidth / 2,
					windowHeight / 2,
					nullptr);

      texs[i] = &depthSlices[i];
      texs[4 + i] = &ssaoSlices[i];
    }

  depthSliceUniformPool.create(1, 1, sizeof(glm::vec4) * 4, false);
  depthSliceUniformPool.addImageArray(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4);
  depthSliceUniformPool.addImageArray(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4);
  depthSliceUniformPool.addBuffer(2, sizeof(glm::vec4) * 4);
  depthSliceUniformPool.createDescriptorSetLayout();

  
  depthSliceUniform.allocateDescriptorSets(&depthSliceUniformPool, &texs[0], nullptr);
glm::vec4* rotMats = (glm::vec4*)depthSliceUniform.getBufferMemoryLocation(0, 0);
  for (int i = 0; i < 4; i++)
    {
      float deg = glm::radians(i / 4.0 * 90);
      rotMats[i] = glm::vec4(
			     glm::cos(deg),
			     -glm::sin(deg),
			     glm::sin(deg),
			     glm::cos(deg)
			     );//normalize?
    }


  std::vector<VkDescriptorSetLayout> depthDeinterleaveLayouts = {SSAOLayout, depthSliceUniformPool.descriptorSetLayout};
  depthDeinterleavePipeline.createPipeline("../src/shaders/SSAODepthDeInterleave.spv",
			  &depthDeinterleaveLayouts, nullptr);
  depthInterleavePipeline.createPipeline("../src/shaders/SSAODepthInterleave.spv",
			  &depthDeinterleaveLayouts, nullptr);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  vkCreateSemaphore(device, &semaphoreInfo, nullptr, &doneDeinterleaving);
  vkCreateSemaphore(device, &semaphoreInfo, nullptr, &interleaveReady);

  std::vector<VkDescriptorSetLayout> SSAOLayouts = {SSAOLayout, cameraLayout, depthSliceUniformPool.descriptorSetLayout};
  VkPushConstantRange pushConstant = {
    VK_SHADER_STAGE_COMPUTE_BIT, 
    0,//  offset
    sizeof(SSAOPushInfo)
  };

  std::vector<VkPushConstantRange> pushes = {pushConstant};
  pipeline.createPipeline("../src/shaders/SSAOMultiPass.spv",
			  &SSAOLayouts, &pushes);

  pushInfo.sigma = 0.51;
  pushInfo.beta = 0.316;
  pushInfo.alpha = 20.0;
  pushInfo.theta = 0.09;

}

void SSAOPass::deinterleaveDepth(std::vector<VkSemaphore> waitSemaphores,
				 std::vector<VkSemaphore> signalSemaphores,
				 VkBTexture* ssaoTex,
			         VkDescriptorSet* deferredUniform)
{
  vkResetCommandBuffer(depthDeinterleavePipeline.commandBuffer, 0);
  //depthDeinterleavePipeline.transitionSampledImageForComputeWrite(ssaoTex->image);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  vkBeginCommandBuffer(depthDeinterleavePipeline.commandBuffer, &beginInfo);
  vkCmdBindPipeline(depthDeinterleavePipeline.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, depthDeinterleavePipeline.pipeline);
  vkCmdBindDescriptorSets(depthDeinterleavePipeline.commandBuffer,
			  VK_PIPELINE_BIND_POINT_COMPUTE, depthDeinterleavePipeline.pipelineLayout,
			  0,1,
			  deferredUniform, 0,0);

  vkCmdBindDescriptorSets(depthDeinterleavePipeline.commandBuffer,
			  VK_PIPELINE_BIND_POINT_COMPUTE, depthDeinterleavePipeline.pipelineLayout,
			  1,1,
			  &depthSliceUniformPool.descriptorSets[0][depthSliceUniform.indexIntoPool], 0,0);

	
  vkCmdDispatch(depthDeinterleavePipeline.commandBuffer, windowWidth / 8, windowHeight / 8,  1); //switch to swapchain width and height

	
  if (vkEndCommandBuffer(depthDeinterleavePipeline.commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
	
  //Syncronization info for compute 
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //Wait for this stage before writing to image
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &depthDeinterleavePipeline.commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores.data();
    

  if (vkQueueSubmit(computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };
  //depthDeinterleavePipeline.transitionImageForComputeSample(ssaoTex->image);  
}
void SSAOPass::interleave(std::vector<VkSemaphore> waitSemaphores,
				 std::vector<VkSemaphore> signalSemaphores,
				 VkBTexture* ssaoTex,
			         VkDescriptorSet* deferredUniform)
{
  vkResetCommandBuffer(depthInterleavePipeline.commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  vkBeginCommandBuffer(depthInterleavePipeline.commandBuffer, &beginInfo);
  vkCmdBindPipeline(depthInterleavePipeline.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, depthInterleavePipeline.pipeline);
  vkCmdBindDescriptorSets(depthInterleavePipeline.commandBuffer,
			  VK_PIPELINE_BIND_POINT_COMPUTE, depthInterleavePipeline.pipelineLayout,
			  0,1,
			  deferredUniform, 0,0);

  vkCmdBindDescriptorSets(depthInterleavePipeline.commandBuffer,
			  VK_PIPELINE_BIND_POINT_COMPUTE, depthInterleavePipeline.pipelineLayout,
			  1,1,
			  &depthSliceUniformPool.descriptorSets[0][depthSliceUniform.indexIntoPool], 0,0);

	
  vkCmdDispatch(depthInterleavePipeline.commandBuffer, windowWidth / 8, windowHeight / 8,  1); //switch to swapchain width and height

	
  if (vkEndCommandBuffer(depthInterleavePipeline.commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
	
  //Syncronization info for compute 
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //Wait for this stage before writing to image
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &depthInterleavePipeline.commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores.data();
    

  if (vkQueueSubmit(computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };
}

void SSAOPass::compute(std::vector<VkSemaphore> waitSemaphores,
		       std::vector<VkSemaphore> signalSemaphores,
		       VkBTexture* albedoTexture,
		       VkBTexture* ssaoTex,
		       VkDescriptorSet* deferredUniform,
		       VkDescriptorSet* cameraUniform
		       )
{

  std::vector<VkSemaphore> signalDeinterleaveDone = {doneDeinterleaving};
  deinterleaveDepth(waitSemaphores, signalDeinterleaveDone, ssaoTex, deferredUniform);

  pipeline.transitionSampledImageForComputeWrite(ssaoTex->image);
  pipeline.transitionSampledImageForComputeWrite(albedoTexture->image);

  vkResetCommandBuffer(pipeline.commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  vkBeginCommandBuffer(pipeline.commandBuffer, &beginInfo);
  vkCmdBindPipeline(pipeline.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline);
  vkCmdBindDescriptorSets(pipeline.commandBuffer,
			  VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipelineLayout,
			  0,1,
			  deferredUniform, 0,0);
  
  vkCmdBindDescriptorSets(pipeline.commandBuffer,
			  VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipelineLayout,
			  1,1,
			  cameraUniform, 0,0);

  vkCmdBindDescriptorSets(pipeline.commandBuffer,
			  VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipelineLayout,
			  2,1,
			  &depthSliceUniformPool.descriptorSets[0][depthSliceUniform.indexIntoPool], 0,0);


  vkCmdPushConstants(pipeline.commandBuffer,
		     pipeline.pipelineLayout,
		     VK_SHADER_STAGE_COMPUTE_BIT,
		     0, sizeof(SSAOPushInfo),
		     &pushInfo);	
	
  vkCmdDispatch(pipeline.commandBuffer, windowWidth / 2 / 8, windowHeight / 2 / 8,  4); 

	
  if (vkEndCommandBuffer(pipeline.commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
	
  //Syncronization info for compute 
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //Wait for this stage before writing to image
  submitInfo.waitSemaphoreCount = 1;
  //submitInfo.pWaitSemaphores = waitSemaphores.data();
  submitInfo.pWaitSemaphores = signalDeinterleaveDone.data();
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  std::vector<VkSemaphore> signalInterleave = {interleaveReady};
  submitInfo.pCommandBuffers = &pipeline.commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalInterleave.data();
    

  if (vkQueueSubmit(computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };

  interleave(signalInterleave, signalSemaphores, ssaoTex, deferredUniform);
  pipeline.transitionImageForComputeSample(ssaoTex->image);
  pipeline.transitionImageForComputeSample(albedoTexture->image);
}


void SSAOPass::destroy()
{
  for (int i = 0; i < 4; i++)
    {
      depthSlices[i].destroy();
      ssaoSlices[i].destroy();
    }
  depthSliceUniformPool.destroy();
  depthSliceUniform.destroy();
  vkDestroySemaphore(device, doneDeinterleaving, nullptr);
  vkDestroySemaphore(device, interleaveReady, nullptr);
  depthDeinterleavePipeline.destroy();
  depthInterleavePipeline.destroy();
  pipeline.destroy();
}
