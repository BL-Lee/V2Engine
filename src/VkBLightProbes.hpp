#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VkBGlobals.hpp"
#include "VkBTexture.hpp"
class VkBLightProbeInfo
{
public:
  glm::vec3 gridDimensions; //total width and height and depth
  glm::vec3 center;
  int resolution; //How many to put along each axis.
  int raysPerProbe;

  int cascadeCount;
  int baseRayCount; //TODO
  int rayIncreasePerCascade; //TODO
  //VkBUniformPool uniformPool;
  //VkBUniformBuffer uniform;
  
  void create();
  void destroy();
  void copyTextureToCPU(VkBTexture tex);
  void transitionImageToStorage(VkImage image);
  void transitionImageToSampled(VkImage image);
};
