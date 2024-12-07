#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "VkBGlobals.hpp"
#include <optional>
#include <vector>

class VkBRenderPass
{
public:
  VkRenderPass renderPass;
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkAttachmentReference> attachmentRefs;
  std::optional<VkAttachmentReference> depthRef;
  void addDepthAttachment(uint32_t ind);
  void addColourAttachment(VkFormat format, bool present, uint32_t ind);
  void createRenderPass();
};
