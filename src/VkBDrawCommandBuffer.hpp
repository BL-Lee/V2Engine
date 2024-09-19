#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "VkBVertexBuffer.hpp"
class VkBDrawCommandBuffer{

public:
  VkCommandBuffer commandBuffer;
  
  void createCommandBuffer(VkCommandPool commandPool);
  void record(VkPipeline pipeline,
	      VkPipelineLayout pipelineLayout,
	      VkBVertexBuffer* vertexBuffer,
	      VkDescriptorSet* descriptorSet,
	      int offset,
	      int count
	      );
  void end();
  void begin(VkRenderPass renderPass,
	     VkFramebuffer frameBuffer,
	     VkExtent2D extent);

};
