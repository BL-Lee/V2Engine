#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include "VkBUniformPool.hpp"
class VkBUniformBuffer
{
  
public:
  /*
 (TODO: static one)
   DYNAMIC:

   HOST_COHERENT buffer(s)

   I Think the pool should give you an offset into a VkBuffer to write into

   Then update ubo-> get offset into buffer -> update that part (repeat) 
                 -> then when the draw call happens it will update the whole thing
		 Since theyre dynamic, they probably will all get updated anyways
		 Yes then when you bind the descriptor set, it already knows the offset and range to load in

   STATIC:
   DEVICE_LOCAL buffer(s)

   Probably medium sized buffers,

   Then when they get updated (like deleting or adding new geometry) it updates that chunk

   But uniformBuffers, uniformBuffersMemory and uniformBuffersMapped should be in UniformPool
   Descriptorsets too probably. Since they all need to be the same anyways.

   I guess try to keep everything with the same uniforms
   (
       material
       model matrix
   )
   

*/
  void* mappedBuffer;
  size_t uniformSize;
  VkBUniformPool* uniformPool;
  int indexIntoPool;

  std::vector<VkDescriptorBufferInfo> bufferInfo;
  std::vector<VkDescriptorImageInfo> imageInfo;
  
  void destroy();
  void createUniformBuffers(size_t uniformSize,
			    int maxFramesInFlight);
  void* getBufferMemoryLocation(int imageIndex, int bufferIndex);
  void allocateDescriptorSets(VkBUniformPool* descriptorPool,
			      VkImageView* textureImageView, //temp
			      VkSampler* textureSampler //temp
);

};
