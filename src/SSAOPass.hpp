#pragma once

#include "VkBComputePipeline.hpp"

struct SSAOPushInfo
{
  float sigma;
  float beta;
  float alpha;
};



class SSAOPass
{
public:
  SSAOPushInfo pushInfo;
  VkBComputePipeline pipeline;
  void init(VkDescriptorSetLayout SSAOLayout,VkDescriptorSetLayout cameraLayout);
  void compute(std::vector<VkSemaphore> waitSemaphores,
	       std::vector<VkSemaphore> signalSemaphores,
	       VkBTexture* albedoTexture,
	       VkDescriptorSet* deferredUniform,
	       VkDescriptorSet* cameraUniform);
  void destroy();
};
