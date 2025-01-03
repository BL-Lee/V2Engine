#pragma once
#include "VkBGlobals.hpp"
#include "VkBComputePipeline.hpp"
#include "VkBUniformPool.hpp"
#include "VkBUniformBuffer.hpp"

struct SSAOPushInfo
{
  float sigma;
  float beta;
  float alpha;
  float theta;
};



class SSAOPass
{
public:
  
  SSAOPushInfo pushInfo;
  VkBComputePipeline pipeline;
  VkBComputePipeline depthDeinterleavePipeline;
  VkBComputePipeline depthInterleavePipeline;
  
  VkSemaphore doneDeinterleaving;
  VkSemaphore interleaveReady;
  
  VkBTexture depthSlices[4];
  VkBTexture ssaoSlices[4];
  VkBUniformPool depthSliceUniformPool;
  VkBUniformBuffer depthSliceUniform;
  
  void init(VkDescriptorSetLayout SSAOLayout,VkDescriptorSetLayout cameraLayout);
  
  void compute(std::vector<VkSemaphore> waitSemaphores,
		       std::vector<VkSemaphore> signalSemaphores,
		       VkBTexture* albedoTexture,
		       VkBTexture* ssaoTex,
		       VkDescriptorSet* deferredUniform,
		       VkDescriptorSet* cameraUniform
		       );
  void deinterleaveDepth(std::vector<VkSemaphore> waitSemaphores,
			 std::vector<VkSemaphore> signalSemaphores,
			 VkDescriptorSet* deferredUniform);
  void interleave(std::vector<VkSemaphore> waitSemaphores,
				 std::vector<VkSemaphore> signalSemaphores,
				 VkBTexture* ssaoTex,
			  VkDescriptorSet* deferredUniform);

  void destroy();
};
