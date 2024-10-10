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

  void create();
  void destroy();
  
};
