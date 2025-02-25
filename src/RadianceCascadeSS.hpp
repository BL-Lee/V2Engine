#pragma once
#include "VkBGlobals.hpp"
#include "VkBLightProbes.hpp"
#include "VkBComputePipeline.hpp"
class RadianceCascadeSS
{
public:
  VkBLightProbeInfo lightProbeInfo;
  VkBComputePipeline lightProbePipeline;
  VkSemaphore finishedSemaphore;
  CascadeInfo cascadeInfos[CASCADE_COUNT];

  void create();
  void destroy();
  void initPipeline(VkBRayInputInfo* rayInputInfo, VkDescriptorSetLayout SSInfoLayout);
  void computeSSRadianceCascade(VkBRayInputInfo* rayInputInfo,
						 std::vector<VkSemaphore> waitSemaphores,
						 std::vector<VkSemaphore> signalSemaphores,
						 VkDescriptorSet* SSInfoBuffer,
						 uint32_t imageIndex
						   );

};
