#pragma once



#include "VkBGlobals.hpp"
#include "VkBTexture.hpp"
#include "glm/glm.hpp"
#include <iostream>
class TextureAtlas
{
public:
  VkBTexture atlas;


  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  void* stagingBufferData;
  
  
  uint32_t width;
  uint32_t height;
  size_t bytesPerPixel;


  uint32_t blockAxisCountW;
  uint32_t blockAxisCountH;
  uint32_t blockWidth;
  uint32_t blockHeight;
  int* occupiedCount;
  std::vector<std::string> occupiedNames;
  void init(uint32_t w, uint32_t h, VkBTextureType type, uint32_t blockSize);

  void transitionAtlasToWrite();

  void transitionAtlasToSample();
  glm::ivec2 checkIfTextureExists(std::string name);
  glm::ivec2 requestAtlasPage();

  void destroy();

  void addToAtlas(void* pixels, glm::ivec2 position, std::string name);

};
