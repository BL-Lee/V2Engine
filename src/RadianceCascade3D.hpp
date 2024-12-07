#pragma once
#include "VkBGlobals.hpp"
#include "VkBLightProbes.hpp"
#include "VkBRayPipeline.hpp"
class RadianceCascade3D
{
public:
  VkBLightProbeInfo lightProbeInfo;
  VkBRayPipeline lightProbePipeline;
  VkSemaphore finishedSemaphore;
  CascadeInfo cascadeInfos[6];

  void create();
  void destroy();
  void initPipeline(VkBRayInputInfo* rayInputInfo);
  void compute3DRadianceCascade(VkBRayInputInfo* rayInputInfo,
				std::vector<VkSemaphore> waitSemaphores,
				std::vector<VkSemaphore> signalSemaphores,
				uint32_t imageIndex
				);

};
