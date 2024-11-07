#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VkBGlobals.hpp"
#include "VkBTexture.hpp"
#include "Vertex.hpp"
#include "VkBVertexBuffer.hpp"

struct CascadeInfo
{
  int cascade, quadrant;
  float start, end;
  int lineViewIndex;
};

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
  VkBTexture textures[4]; //Unhardcode
  VkSemaphore semaphoreChain[4]; //unhardcode
  size_t imageWidth;
  //VkBUniformPool uniformPool;
  //VkBUniformBuffer uniform;

  //DEBUG LINES------------------
  VkBuffer lineBuffer; 
  VkDeviceMemory lineBufferMemory;
  void* lineBufferMapped;
  LineVertex* lines;
  VkBVertexBuffer lineVBO;
  int lineCount;
  int debugCascadeViewIndex;
  int debugDirectionViewIndex;
  int viewDebug;
  
  
  
  void create();
  void destroy();
  //  void copyTextureToCPU(VkBTexture* tex);
  void transitionImageToStorage(VkImage image);
  void transitionImageToSampled(VkImage image);
  //  void processLightProbeTextureToLines();//deprecated. Fills line buffer in compute shader now
};
