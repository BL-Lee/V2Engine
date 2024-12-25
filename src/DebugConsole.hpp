#pragma once
#include "VkBGlobals.hpp"
#include "swapChain.hpp"
#include "VkBLightProbes.hpp"
#include "VkBComputePipeline.hpp"
#include "SSAOPass.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include "implot_internal.h"
#define GPU_TIMINGS_ALLOC_SIZE 100
class DebugConsole
{
public:

  
  // ImGUI Data
  VkAllocationCallbacks*   g_Allocator = nullptr;
  VkInstance               g_Instance = VK_NULL_HANDLE;
  //We already handled these
  //static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE; 
  //static VkDevice                 g_Device = VK_NULL_HANDLE;
  //static uint32_t                 g_QueueFamily = (uint32_t)-1;
  //static VkQueue                  g_Queue = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
  VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
  VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

  ImGui_ImplVulkanH_Window g_MainWindowData;
  int                      g_MinImageCount = 2;
  bool                     g_SwapChainRebuild = false;
  
  //Other
  bool show = false;
  
  CascadeInfo* cascadeInfos[CASCADE_COUNT];
  SSAOPushInfo* ssaoInfo;
  RayDebugPushConstant* rayDebugPushConstant;

  uint32_t timingFrame;
  double* totalGPUTime;
  double* SSAOTime;
  double* deferredTime;
  double* compositeTime;
  double* reflectTime;
  double* xs;
  std::vector<uint64_t> timeStamps;

  VkQueryPool queryPoolTimeStamps;
  
  void init(VkBSwapChain* swapChain, VkRenderPass renderPass);
  void destroy();
  void gatherTimings();
  void draw(VkCommandBuffer drawCommandBuffer);
};
