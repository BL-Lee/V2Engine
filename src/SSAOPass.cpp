#include "SSAOPass.hpp"
#include <vector>
#include <stdexcept>
void SSAOPass::init(VkDescriptorSetLayout SSAOLayout,
		    VkDescriptorSetLayout cameraLayout)
{
  std::vector<VkDescriptorSetLayout> SSAOLayouts = {SSAOLayout, cameraLayout};
  VkPushConstantRange pushConstant = {
    VK_SHADER_STAGE_COMPUTE_BIT, 
    0,//  offset
    sizeof(SSAOPushInfo)
  };

  std::vector<VkPushConstantRange> pushes = {pushConstant};
  pipeline.createPipeline("../src/shaders/SSAO.spv",
			  &SSAOLayouts, &pushes);

  pushInfo.sigma = 0.51;
  pushInfo.beta = 0.316;
  pushInfo.alpha = 15.0;
}

void SSAOPass::compute(std::vector<VkSemaphore> waitSemaphores,
		       std::vector<VkSemaphore> signalSemaphores,
		       VkBTexture* albedoTexture,
		       VkDescriptorSet* deferredUniform,
		       VkDescriptorSet* cameraUniform
		       )
{
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

  vkCmdPushConstants(pipeline.commandBuffer,
		     pipeline.pipelineLayout,
		     VK_SHADER_STAGE_COMPUTE_BIT,
		     0, sizeof(SSAOPushInfo),
		     &pushInfo);	
	
  vkCmdDispatch(pipeline.commandBuffer, albedoTexture->width / 8, albedoTexture->height / 8,  1); //switch to swapchain width and height

	
  if (vkEndCommandBuffer(pipeline.commandBuffer) != VK_SUCCESS) {
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
  submitInfo.pCommandBuffers = &pipeline.commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores.data();
    

  if (vkQueueSubmit(computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };
  pipeline.transitionImageForComputeSample(albedoTexture->image);
}


void SSAOPass::destroy()
{
  pipeline.destroy();
}
