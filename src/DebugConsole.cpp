#include "DebugConsole.hpp"
#include <stdexcept>
void DebugConsole::init(VkBSwapChain* swapChain, VkRenderPass renderPass)
{
  totalGPUTime = (double*)calloc(sizeof(double), GPU_TIMINGS_ALLOC_SIZE);
  SSAOTime = (double*)calloc(sizeof(double), GPU_TIMINGS_ALLOC_SIZE);
  deferredTime = (double*)calloc(sizeof(double), GPU_TIMINGS_ALLOC_SIZE);
  compositeTime = (double*)calloc(sizeof(double), GPU_TIMINGS_ALLOC_SIZE);
  reflectTime = (double*)calloc(sizeof(double), GPU_TIMINGS_ALLOC_SIZE);
  xs = (double*)calloc(sizeof(double), GPU_TIMINGS_ALLOC_SIZE);
  timingFrame = 0;
  timeStamps = std::vector<uint64_t>(2);

  for (int i = 0; i < GPU_TIMINGS_ALLOC_SIZE; i++)
    {
      xs[i] = i;
    }

  VkQueryPoolCreateInfo queryPoolInfo{};
  queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
  queryPoolInfo.queryCount = 2;
  if (vkCreateQueryPool(device, &queryPoolInfo, nullptr, &queryPoolTimeStamps) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create timer query pool");
    }

  
  // Create Descriptor Pool
  // The example only requires a single combined image sampler descriptor for the font image and only uses one descriptor set (for that)
  // If you wish to load e.g. additional textures you may need to alter pools sizes.
  {
    VkDescriptorPoolSize pool_sizes[] =
      {
	{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
      };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(device, &pool_info, g_Allocator, &g_DescriptorPool);
  }


  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = instance;
  init_info.PhysicalDevice = physicalDevice;
  init_info.Device = device;
  init_info.QueueFamily = graphicsQueueFamily;
  init_info.Queue = graphicsQueue;
  init_info.PipelineCache = nullptr;
  init_info.DescriptorPool = g_DescriptorPool;
  init_info.RenderPass = renderPass;
  init_info.Subpass = 0;
  init_info.MinImageCount = g_MinImageCount;
  init_info.ImageCount = swapChain->images.size();
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = nullptr;
  ImGui_ImplVulkan_Init(&init_info);

  
    
}
void DebugConsole::destroy()
{


  free(totalGPUTime);
  free(SSAOTime);
  free(deferredTime);
  free(compositeTime);
  free(reflectTime);
  free(xs);
  vkDestroyQueryPool(device, queryPoolTimeStamps, nullptr);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  vkDestroyDescriptorPool(device, g_DescriptorPool, nullptr);
}

void DebugConsole::gatherTimings()
{
  vkGetQueryPoolResults(device,
			queryPoolTimeStamps,
			0,
			2,
			2 * sizeof(uint64_t),
			timeStamps.data(),
			sizeof(uint64_t),
			VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);



  totalGPUTime[timingFrame] = (timeStamps[1] - timeStamps[0]) * physicalDeviceProperties.limits.timestampPeriod / 1000000.0f;;
  timingFrame = (timingFrame + 1) % GPU_TIMINGS_ALLOC_SIZE;
}
void DebugConsole::draw(VkCommandBuffer drawCommandBuffer)
{
  // Start the Dear ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  bool show_another_window = true;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

  if (ImGui::CollapsingHeader("Cascade Info"))
    {

      //  if (ImGui::SliderFloat("bilateral Blend", &cascadeInfos[0]->bilateralBlend, 0.0, 10000.1)) {
      if (ImGui::SliderFloat("bilateral Blend", &cascadeInfos[0]->bilateralBlend, 0.0, 20.0)) {
	for (int i =1 ; i < CASCADE_COUNT; i++)
	  {
	    cascadeInfos[i]->bilateralBlend = cascadeInfos[0]->bilateralBlend;
	  }
      }



      for (int i = 0; i < CASCADE_COUNT; i++)
	{
	  ImGui::PushID(i);

	  if (ImGui::SliderFloat("Cascade Distance", &cascadeInfos[i]->end, 0.0, 2.0)) {
	    if (i == 0)
	      {
		for (int j = 1; j < CASCADE_COUNT; j++)
		  {
		    cascadeInfos[j]->start = cascadeInfos[j-1]->end;
		    cascadeInfos[j]->end = cascadeInfos[j]->start * 5.0;
		  }
	      }

	  }
	  ImGui::PopID();
	}
    }
  if (ImGui::CollapsingHeader("SSAO Info"))
    {
      ImGui::SliderFloat("SSAO sigma", &ssaoInfo->sigma, 0.0, 2.0);
      ImGui::SliderFloat("SSAO beta", &ssaoInfo->beta, 0.0, 2.0);
      ImGui::SliderFloat("SSAO alpha", &ssaoInfo->alpha, 0.0, 20.0);
    }
  if (ImGui::CollapsingHeader("Ray Info"))
    {
      static const char* items[] = {"0","1","2","3"};
      static int selectedItem = 0;
      ImGui::Combo("MyCombo", &selectedItem, items, IM_ARRAYSIZE(items));
      rayDebugPushConstant->viewMode = (uint32_t)selectedItem;
  
      ImGui::SliderFloat("RayTriangleLimit", &rayDebugPushConstant->triangleTestLimit, 0.0, 300.0);
      ImGui::SliderFloat("RayBoxLimit", &rayDebugPushConstant->boxTestLimit, 0.0, 10.0);
    }
  if (ImGui::CollapsingHeader("GPU Profilings"))
    {
      static ImPlotShadedFlags flags = 0;
      if (ImPlot::BeginPlot("TimingsPlot")) {
	ImPlot::PlotLine("Stock 1", xs, totalGPUTime, GPU_TIMINGS_ALLOC_SIZE);
	ImPlot::EndPlot();
      }
    }
  
  
  ImGui::End();
  
  //ImGui::ShowDemoWindow(); // Show demo window! :)
 

  
  // Rendering
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  // Record dear imgui primitives into command buffer
  ImGui_ImplVulkan_RenderDrawData(drawData, drawCommandBuffer);

}
