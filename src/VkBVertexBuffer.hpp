#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "Vertex.hpp"
//Should be renamed, I'm gonna keep indices in here too
class VkBVertexBuffer
{
public:

  /*
    Using two buffers 
    
    Staging buffer:
    - CPU side, for writing to on CPU, 

    Other buffer:
    - GPU side

    This way we can send the entire staging buffer all at once, instead of in pieces
    Having a buffer on dedicated VRAM also helps it optimize better
    Right now im using a single staging buffer and offsetting into that for transfers,
    but that can change later

https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
 Driver developers recommend that you also store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because it's closer together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used during the same render operations, provided that their data is refreshed, of course. This is known as aliasing and some Vulkan functions have explicit flags to specify that you want to do this.
   */
  
  
  VkBuffer buffer;
  VkBuffer indexBuffer;
  VkBuffer stagingBuffer;
  //VkMemoryRequirements memRequirements;
  VkDeviceMemory bufferMemory;
  VkDeviceMemory stagingBufferMemory;
  VkDeviceMemory indexBufferMemory;

  
  uint32_t vertexCount;
  uint32_t indexCount;
  
  void create(VkDevice device, VkPhysicalDevice physicalDevice, size_t initialVertexSize, size_t initialIndexSize)  ;
  void destroy(VkDevice device);
  void fill(VkDevice device,
	    const Vertex* vertices, uint32_t vertexCount,
	    const uint32_t* indices, uint32_t indexCount);
  void transferToDevice(VkDevice device, VkCommandPool transientPool, VkQueue graphicsQueue);
};