#include "RadianceCascadeSS.hpp"
#include <stdexcept>

void RadianceCascadeSS::create()
{
  for (int i = 0; i < CASCADE_COUNT; i++)
    {
      if (i == 0)
	cascadeInfos[i] = {i, 0, 15.0f, 0.0f, 0.02f, 0};
      else
	cascadeInfos[i] = {i, 0, 15.0f,  cascadeInfos[i-1].end, cascadeInfos[i-1].end * 5.0f,0};

    }
   lightProbeInfo.create(framesInFlight, true, glm::vec3(800,800,0));
}

void RadianceCascadeSS::initPipeline(VkBRayInputInfo* rayInputInfo, VkDescriptorSetLayout SSInfoLayout)
{
  VkPushConstantRange cascadePushConstant = {
    VK_SHADER_STAGE_COMPUTE_BIT, 
    0,//  offset
    sizeof(CascadeInfo)
  };

  std::vector<VkPushConstantRange> cascadePushConstantRange = {cascadePushConstant};

  std::vector<VkDescriptorSetLayout> probeUniformLayouts = {rayInputInfo->assemblerPool.descriptorSetLayout,
							    lightProbeInfo.computeUniformPool.descriptorSetLayout, SSInfoLayout};
  
  lightProbePipeline.createPipeline("../src/shaders/SSRadianceCascade.spv",
				    &probeUniformLayouts, &cascadePushConstantRange,
				    lightProbeInfo.cascadeCount);
}

void RadianceCascadeSS::computeSSRadianceCascade(VkBRayInputInfo* rayInputInfo,
						 std::vector<VkSemaphore> waitSemaphores,
						 std::vector<VkSemaphore> signalSemaphores,
						 VkDescriptorSet* SSInfoBuffer,
						 uint32_t imageIndex
						 )
{
  lightProbeInfo.transitionImagesToStorage();
  for (int i = lightProbeInfo.cascadeCount - 1; i >= 0 ; i--)
    {
      vkResetCommandBuffer(lightProbePipeline.commandBuffers[i], 0);

      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = 0; // Optional
      beginInfo.pInheritanceInfo = nullptr; // Optional

      vkBeginCommandBuffer(lightProbePipeline.commandBuffers[i], &beginInfo);
	
      vkCmdBindPipeline(lightProbePipeline.commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, lightProbePipeline.pipeline);
	
      vkCmdBindDescriptorSets(lightProbePipeline.commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE,
			      lightProbePipeline.pipelineLayout,
			      0, 1,
			      &rayInputInfo->assemblerPool.descriptorSets[imageIndex][rayInputInfo->assemblerBuffer.indexIntoPool]
			      , 0, 0);
      vkCmdBindDescriptorSets(lightProbePipeline.commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE,
			      lightProbePipeline.pipelineLayout,
			      1, 1,
			      &lightProbeInfo.computeUniformPool.descriptorSets[imageIndex][lightProbeInfo.computeUniforms[i].indexIntoPool]
			      , 0, 0);

      vkCmdBindDescriptorSets(lightProbePipeline.commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE,
			      lightProbePipeline.pipelineLayout,
			      2, 1,
			      SSInfoBuffer,
			       0, 0);


      //Cascade info cascade 0
      vkCmdPushConstants(lightProbePipeline.commandBuffers[i],
			 lightProbePipeline.pipelineLayout,
			 VK_SHADER_STAGE_COMPUTE_BIT,
			 0, sizeof(CascadeInfo),
			 &cascadeInfos[i]);	

      vkCmdDispatch(lightProbePipeline.commandBuffers[i],
		    lightProbeInfo.textures[i].width / 8,
		    lightProbeInfo.textures[i].height / 8,
		    1);
	
      if (vkEndCommandBuffer(lightProbePipeline.commandBuffers[i]) != VK_SUCCESS) {
	throw std::runtime_error("failed to record command buffer!");
      }
	
      //Syncronization info for compute 
      VkSubmitInfo submitInfo{};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      VkSemaphore* waitChainSemaphores;
      VkSemaphore* signalChainSemaphores;
      if (i == lightProbeInfo.cascadeCount - 1)
	{
	  waitChainSemaphores = waitSemaphores.data(); //Wait for image availble, ie prev frame done
	  signalChainSemaphores = &lightProbeInfo.semaphoreChain[i]; //Signal to next chain ready
	}
      else if (i == 0)
	{
	  waitChainSemaphores = &lightProbeInfo.semaphoreChain[i + 1]; //Wait for previous cascade
	  signalChainSemaphores = signalSemaphores.data(); //Signal that cascades are done
	}
      else
	{
	  waitChainSemaphores = &lightProbeInfo.semaphoreChain[i + 1]; //Wait previous cascade
	  signalChainSemaphores = &lightProbeInfo.semaphoreChain[i]; //Signal to next cascade
	}
      VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT}; //Wait for this stage before writing to image

      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = waitChainSemaphores;
      submitInfo.pWaitDstStageMask = waitStages;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &lightProbePipeline.commandBuffers[i];
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = signalChainSemaphores;
    

      if (vkQueueSubmit(computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
	throw std::runtime_error("failed to submit compute command buffer!");
      };

      lightProbeInfo.transitionImageToSampled(i);
    }

}
