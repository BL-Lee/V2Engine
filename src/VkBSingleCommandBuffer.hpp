#pragma once
#include "VkBGlobals.hpp"
VkCommandBuffer vKBeginSingleTimeCommandBuffer();
void vKEndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer);
