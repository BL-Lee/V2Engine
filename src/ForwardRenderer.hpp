#pragma once
#include "VkBGlobals.hpp"
#include "VkBDrawCommandBuffer.hpp"
#include "VkBGraphicsPipeline.hpp"
#include "VkBRenderPass.hpp"
#include "VkBVertexBuffer.hpp"
#include "OBJLoader.hpp"
#include <vector>
class ForwardRenderer
{
public:
  VkBDrawCommandBuffer drawCommandBuffer;
  VkBGraphicsPipeline currentPipeline;
  VkBRenderPass renderPass;
  void begin( VkExtent2D extent, VkFramebuffer framebuffer,
	      VkBGraphicsPipeline pipline
	      );
  void bindDescriptorSet( VkDescriptorSet* dSet, uint32_t ind );
  void record(VkBVertexBuffer* vertexBuffer, Model* model);
  void record(VkBVertexBuffer* vertexBuffer, uint32_t start, uint32_t stop);
  void changePipeline(VkBGraphicsPipeline pipeline);
  void submit(  VkSubmitInfo* submitInfo, VkFence fence);

};
