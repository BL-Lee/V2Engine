#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VkBGlobals.hpp"
#include "VkBTexture.hpp"
#include "Vertex.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBUniformPool.hpp"
#include "VkBUniformBuffer.hpp"
#include "VkBRayPipeline.hpp"
#include "VkBRayInputInfo.hpp"
struct CascadeInfo
{
  int cascade, quadrant;
  float bilateralBlend;
  float start, end;
  int lineViewIndex;
};

class VkBLightProbeInfo
{
public:
  glm::vec3 gridDimensions; //total width and height and depth
  glm::vec3 center;
  //int resolution; //How many to put along each axis.
  
  int raysPerProbe;

  int cascadeCount;
  int baseRayCount; //TODO
  int rayIncreasePerCascade; //TODO
  VkBTexture* textures;
  VkSemaphore* semaphoreChain;
  
  size_t imageWidth;
  size_t imageHeight;
  size_t imageDepth;

  
  VkBUniformPool computeUniformPool;
  VkBUniformBuffer* computeUniforms;
  VkBUniformPool drawUniformPool;
  VkBUniformBuffer drawUniform;
  

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
  
  void create(int frameCount, bool screenSpace, glm::vec3 resolution);
  void destroy();
  void transitionImagesToStorage();
  void transitionImageToSampled(int cascadeIndex);

};
