#include "VkBLightProbes.hpp"
void VkBLightProbeInfo::create()
  {
    resolution = 10;
    gridDimensions = glm::vec3(2.0f,2.0f,2.0f);
    center = glm::vec3(0.0f,0.0f,0.0f);
    raysPerProbe = 20;
  }
void VkBLightProbeInfo::destroy() {
}
