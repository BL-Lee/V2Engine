#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <limits>
#include <cstdint>
#include <algorithm>
#include <optional>
#include <set>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <chrono>

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 800;

std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};
std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
VkInstance instance = VK_NULL_HANDLE;  

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkPhysicalDeviceProperties physicalDeviceProperties;
VkDevice device = VK_NULL_HANDLE;
VkQueue graphicsQueue;
uint32_t graphicsQueueFamily;
VkQueue presentQueue;
VkQueue computeQueue;
VkCommandPool drawCommandPool;
VkCommandPool transientCommandPool; //For short lived command buffers
VkCommandPool computeCommandPool;
uint32_t framesInFlight;
int USE_RASTER = 1; //Swap between ray and raster
int DRAW_DEBUG_LINES = 0;
int viewCascade = 0;

GLFWwindow* window;
double mouseX = 0;
double mouseY = 0;
bool cameraEnabled = 0;


#define NDEBUG_MODE 0
#if NDEBUG_MODE
const bool enableVulkanValidationLayers = false;
#else
const bool enableVulkanValidationLayers = true;
#endif
#include "VkBGlobals.hpp"
#include "vkDebug.hpp"
#include "swapChain.hpp"
#include "DeviceSelection.hpp"
#include "VkBGraphicsPipeline.hpp"
#include "VkBLineGraphicsPipeline.hpp"
#include "VkBRenderPass.hpp"
#include "VkBCommandPool.hpp"
#include "VkBSingleCommandBuffer.hpp"
#include "VkBDrawCommandBuffer.hpp"
#include "VkBVertexBuffer.hpp"
#include "VkBUniformBuffer.hpp"
#include "VkBUniformPool.hpp"
#include "VkBTexture.hpp"
#include "VkBShader.hpp"
#include "OBJLoader.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "VkBLightProbes.hpp"
#include "VkBRayPipeline.hpp"
#include "DebugConsole.hpp"
#include "VkBRayInputInfo.hpp"
#include "BVH.hpp"
Input inputInfo = {};
DebugConsole debugConsole;
CascadeInfo cascadeInfos[4];
RayDebugPushConstant rayDebugPushConstant;

class V2Engine {
public:
  

  VkSurfaceKHR surface;

  VkBGraphicsPipeline graphicsPipeline;
  VkBLineGraphicsPipeline linePipeline;
  VkBRayPipeline rayPipeline;
  VkBRayPipeline lightProbePipeline;
  
  VkBRenderPass renderPass;

  VkBVertexBuffer staticVertexBuffer;
  VkBUniformPool matrixPool;

  VkBRayInputInfo rayInputInfo;
  VkBTexture rayBackBuffer;
  
  VkBUniformPool cameraUniformPool;
  Camera mainCamera;
  VkBDrawCommandBuffer drawCommandBuffer;
  VkBTexture depthTexture;
  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkSemaphore probeInfoFinishedSemaphore;
  VkFence inFlightFence;

  Model* ratModel;
  Model* cornellScene;
  Model* cornellLeftWall;
  Model* cornellRightWall;
  Model* cornellLight;
  Model* cornellCeilLight;
  Material emissive;
  Material diffuseGreen;
  Material diffuseRed;
  Material diffuseGrey;

  VkBLightProbeInfo lightProbeInfo;
  
  
  std::chrono::time_point<std::chrono::high_resolution_clock> fpsPrev;

  void run() {
    fpsPrev = std::chrono::high_resolution_clock::now();
#if NDEBUG_MODE
    std::cout << "V2 Engine: Release Mode" << std::endl;
#else
    std::cout << "V2 Engine: Debug Mode" << std::endl;
#endif
    
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  
  DebugUtils debugUtils;
  VkBSwapChain swapChain;
  

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    if (action == GLFW_PRESS)
      inputInfo.keysPressed[key] = 1;
    if (action == GLFW_RELEASE)
      inputInfo.keysPressed[key] = 0;
    if (action == GLFW_PRESS && key == GLFW_KEY_R)
      USE_RASTER = !USE_RASTER;
    if (action == GLFW_PRESS && key == GLFW_KEY_T)
      cameraEnabled = !cameraEnabled;
    
    if (action == GLFW_PRESS && key == GLFW_KEY_U)
      DRAW_DEBUG_LINES = !DRAW_DEBUG_LINES;
    
    if (action == GLFW_PRESS && key == GLFW_KEY_I)
      {
	viewCascade = (viewCascade + 1) % (3); //cascade count
        cascadeInfos[0].lineViewIndex = viewCascade;
	cascadeInfos[1].lineViewIndex = viewCascade;
	cascadeInfos[2].lineViewIndex = viewCascade;
	cascadeInfos[3].lineViewIndex = viewCascade;
      }
    if (action == GLFW_PRESS && key == GLFW_KEY_O)
      cascadeInfos[viewCascade].quadrant = (cascadeInfos[viewCascade].quadrant + 1) % (viewCascade == 0 ? 8 : 64);
    if (action == GLFW_PRESS && key == GLFW_KEY_P)
      cascadeInfos[viewCascade].quadrant = (cascadeInfos[viewCascade].quadrant - 1) % (viewCascade == 0 ? 8 : 64);
    //viewAllDirections = !viewAllDirections;
    if (action == GLFW_PRESS && key == GLFW_KEY_GRAVE_ACCENT)
      {
	if (debugConsole.show)
	  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
	  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	debugConsole.show = !debugConsole.show;
      }


    if (action == GLFW_PRESS && key == GLFW_KEY_Q)
      glfwSetWindowShouldClose(window, GL_TRUE);
  }

  void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Triangle", nullptr, nullptr);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(window, &mouseX, &mouseY);
  }
  
  void initVulkan() {
    createInstance();
#if !NDEBUG_MODE
      debugUtils.setupDebugMessenger(instance);
#endif
    createSurface();
    physicalDevice = VkBDeviceSelection::pickPhysicalDevice(instance, surface, deviceExtensions);
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkBDeviceSelection::printPhysicalDeviceProperties(&physicalDeviceProperties);

    
    createLogicalDevice();

    //Swap chain and pipeilne
    swapChain.createSwapChain(surface, window);
    swapChain.createImageViews();
    framesInFlight = (uint32_t)swapChain.imageViews.size();
    renderPass.createRenderPass(swapChain.imageFormat);
        //Command pools
    createCommandPool(&drawCommandPool, device, physicalDevice, surface,
		      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    createCommandPool(&transientCommandPool, device, physicalDevice, surface,
		      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    createCommandPool(&computeCommandPool, device, physicalDevice, surface,
		      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    rayBackBuffer.createTextureImage(VKB_TEXTURE_TYPE_STORAGE_RGBA,
				      swapChain.extent.width,
				      swapChain.extent.height,
				      nullptr);

    
    matrixPool.create(6, (uint32_t)swapChain.imageViews.size(), sizeof(glm::mat4));
    matrixPool.addBuffer(0, sizeof(glm::mat4));
    matrixPool.addImage(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    matrixPool.createDescriptorSetLayout();
    cascadeInfos[0] = {0, 0, 0.0f, 0.002f};
    cascadeInfos[1] = {1, 0, 0.005f, 0.16f};
    cascadeInfos[2] = {2, 0, 0.16f, 1.2f};
    cascadeInfos[3] = {3, 0, 1.2f, 1000.0f}; 
    
    cameraUniformPool.create(1, (uint32_t)swapChain.imageViews.size(), sizeof(glm::mat4)*3 + sizeof(float)*4);
    cameraUniformPool.addBuffer(1, sizeof(glm::mat4)*3 + sizeof(float)*4);
    cameraUniformPool.createDescriptorSetLayout();

    mainCamera.init();
    mainCamera.createPerspective((float)swapChain.extent.width, (float)swapChain.extent.height);
    mainCamera.ubo.allocateDescriptorSets(&cameraUniformPool, nullptr, nullptr, nullptr);
    
    
    lightProbeInfo.create((uint32_t)swapChain.imageViews.size());
    std::vector<VkDescriptorSetLayout> uniformLayouts = {matrixPool.descriptorSetLayout, cameraUniformPool.descriptorSetLayout, lightProbeInfo.drawUniformPool.descriptorSetLayout};
    VkPushConstantRange cascadePushConstant = {
      VK_SHADER_STAGE_FRAGMENT_BIT, 
      0,//  offset
      sizeof(CascadeInfo)
    };

    std::vector<VkPushConstantRange> cascadePushConstantRange = {cascadePushConstant};
    graphicsPipeline.createGraphicsPipeline(swapChain, renderPass, &uniformLayouts, &cascadePushConstantRange);
    cascadePushConstantRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    linePipeline.createLineGraphicsPipeline(swapChain, renderPass, &uniformLayouts, &cascadePushConstantRange);


    depthTexture.createTextureImage(VKB_TEXTURE_TYPE_DEPTH, swapChain.extent.width, swapChain.extent.height, nullptr);    
    swapChain.createFramebuffers(renderPass, depthTexture.imageView);
    std::cout << "Swap Chain image count: " << swapChain.imageViews.size() << std::endl;
    
    rayInputInfo.init();
    emissive.index = 0;
    diffuseGrey.index = 1;
    diffuseGreen.index = 2;
    diffuseRed.index = 3;
    cornellScene = ModelImporter::loadOBJ("../models/cornell_nowalls.obj", "../models/cornell.png", &rayInputInfo.vertexBuffer, &diffuseGrey);
    cornellRightWall = ModelImporter::loadOBJ("../models/cornell_right_wall.obj", "../models/cornell.png", &rayInputInfo.vertexBuffer, &diffuseGreen);
    cornellLeftWall = ModelImporter::loadOBJ("../models/cornell_left_wall.obj", "../models/cornell.png", &rayInputInfo.vertexBuffer, &diffuseRed);
    cornellLight = ModelImporter::loadOBJ("../models/cornell_light.obj", "../models/cornell.png", &rayInputInfo.vertexBuffer, &emissive);
    cornellCeilLight = ModelImporter::loadOBJ("../models/cornell_light_ceil.obj", "../models/cornell.png", &rayInputInfo.vertexBuffer, &emissive);
    ratModel = ModelImporter::loadOBJ("../models/rat.obj", "../models/rat.png", &rayInputInfo.vertexBuffer, &emissive);
	  
    VkImageView ratViews[] = {ratModel->textures.imageView};
    VkSampler ratSamplers[] = {ratModel->textures.textureSampler};
    ratModel->modelUniform.allocateDescriptorSets(&matrixPool, ratViews, ratSamplers, nullptr);
    VkImageView cornellViews[] = {cornellScene->textures.imageView};
    VkSampler cornellSamplers[] = {cornellScene->textures.textureSampler};

    cornellScene->modelUniform.allocateDescriptorSets(&matrixPool, cornellViews, cornellSamplers, nullptr);
    cornellLight->modelUniform.allocateDescriptorSets(&matrixPool, cornellViews, cornellSamplers, nullptr);
    cornellCeilLight->modelUniform.allocateDescriptorSets(&matrixPool, cornellViews, cornellSamplers, nullptr);
    cornellLeftWall->modelUniform.allocateDescriptorSets(&matrixPool, cornellViews, cornellSamplers, nullptr);
    cornellRightWall->modelUniform.allocateDescriptorSets(&matrixPool, cornellViews, cornellSamplers, nullptr);
    rayInputInfo.addModel(cornellScene);
    rayInputInfo.addModel(cornellLight);
    rayInputInfo.addModel(cornellCeilLight);
    rayInputInfo.addModel(cornellLeftWall);
    rayInputInfo.addModel(cornellRightWall);
    rayInputInfo.addModel(ratModel);
    rayInputInfo.bvh.transferBVHData();

    
    VkBuffer storageBuffers[] = {rayInputInfo.vertexBuffer.vertexBuffer, rayInputInfo.vertexBuffer.indexBuffer, rayInputInfo.bvh.deviceBuffer, rayInputInfo.deviceMatrixBuffer};
    rayInputInfo.assemblerBuffer.allocateDescriptorSets(&rayInputInfo.assemblerPool,
						     &rayBackBuffer.imageView,
						     &rayBackBuffer.textureSampler,
						     storageBuffers);
    
    std::vector<VkDescriptorSetLayout> computeUniformLayouts = {rayInputInfo.assemblerPool.descriptorSetLayout,
								cameraUniformPool.descriptorSetLayout};
    VkPushConstantRange rayPushConstant = {
      VK_SHADER_STAGE_COMPUTE_BIT, 
      0,//  offset
      sizeof(RayDebugPushConstant)
    };

    std::vector<VkPushConstantRange> rayPushConstantRange = {rayPushConstant};
    rayPipeline.createPipeline("../src/shaders/ray.spv", &computeUniformLayouts, &rayPushConstantRange);
    std::vector<VkDescriptorSetLayout> probeUniformLayouts = {rayInputInfo.assemblerPool.descriptorSetLayout,
							      lightProbeInfo.computeUniformPool.descriptorSetLayout};
    cascadePushConstantRange[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    lightProbePipeline.createPipeline("../src/shaders/rayProbe.spv",
				      &probeUniformLayouts, &cascadePushConstantRange, lightProbeInfo.cascadeCount);
    drawCommandBuffer.createCommandBuffer(drawCommandPool);
    createSyncObjects();
    debugConsole.init(&swapChain, renderPass.renderPass);
    for (int i = 0; i < 4; i++)
      debugConsole.cascadeInfos[i] = &cascadeInfos[i];
    debugConsole.rayDebugPushConstant = &rayDebugPushConstant;
    rayDebugPushConstant.viewMode = 1;
    rayDebugPushConstant.triangleTestLimit = 140;
    rayDebugPushConstant.boxTestLimit = 3;

  }

  void createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //Create it so it is already ready first time we wait for it
    

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
	vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
	vkCreateSemaphore(device, &semaphoreInfo, nullptr, &probeInfoFinishedSemaphore) != VK_SUCCESS ||
	vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
      throw std::runtime_error("failed to create semaphores!");
    }
  }
  
  //EXTENSIONS ----------------------------------------------------
  std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableVulkanValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);      
    }
    
    return extensions;
  }
  

  //INSTANCE -------------------------------------------------
  void createInstance() {

    //Make sure validation layers are on
    if (enableVulkanValidationLayers && !debugUtils.checkValidationLayerSupport(validationLayers)) {
      throw std::runtime_error("Validation layers requested, but not available");
    }
    
    
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableVulkanValidationLayers) {
      createInfo.enabledLayerCount = (uint32_t)(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
      debugUtils.populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }


    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create vulkan instance");
    }
      
  }

  void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create window surface");
    }
  }

  
  //Logical device -------------------------------------------------
  void createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
      indices.graphicsFamily.value(), indices.presentFamily.value()
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1; //Only really need one, unless we're submitting to them on multiple threads CPU side
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{}; //The features we'll be using. Default everything to false for now
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();

    if (enableVulkanValidationLayers) {
      createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)  {
      throw std::runtime_error("Failed to create logical device");
    }
    graphicsQueueFamily = indices.graphicsFamily.value();
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
    
    
  }
  //Queue families -----------------------------------------------------


  void updateProjectionMatrices(uint32_t currentImage)
  {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


    glm::mat4 modelMat = glm::scale(glm::translate(glm::mat4(1.0f),
					  glm::vec3(0.0, glm::sin(time), 0.0)),
			   glm::vec3(0.5f, 0.5f, 0.5f));
    
    memcpy(ratModel->modelUniform.getBufferMemoryLocation(currentImage,0), &modelMat, sizeof(glm::mat4));
    ratModel->modelMatrix = modelMat;
    
    glm::mat4 identity = glm::mat4(1.0f);
    memcpy(cornellScene->modelUniform.getBufferMemoryLocation(currentImage,0), &identity, sizeof(glm::mat4));
    memcpy(cornellLight->modelUniform.getBufferMemoryLocation(currentImage,0), &identity, sizeof(glm::mat4));
    memcpy(cornellCeilLight->modelUniform.getBufferMemoryLocation(currentImage,0), &identity, sizeof(glm::mat4));
    memcpy(cornellLeftWall->modelUniform.getBufferMemoryLocation(currentImage,0), &identity, sizeof(glm::mat4));
    memcpy(cornellRightWall->modelUniform.getBufferMemoryLocation(currentImage,0), &identity, sizeof(glm::mat4));

    mainCamera.updateMatrices(currentImage);
    rayInputInfo.updateModelMatrix(ratModel);
    //std::cout << (uint32_t)ratModel->indexIntoModelMatrixBuffer << std::endl;
    //std::cout << rayInputInfo.matrixCount << std::endl;
    rayInputInfo.transferMatrixData();
  }
  
  void drawFrame() {

    //Wait for previous frame to finish
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence); //Reset it to unsignaled

    //Get image index we'll draw to, indicating the semaphore for the presentation engine to signal when its done using it. After that we can write to it
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain.swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    updateProjectionMatrices(imageIndex);
    //Radiance Cascades ------------------------------------------------------------
    if (USE_RASTER)
      {
	lightProbeInfo.transitionImagesToStorage();
	for (int i = lightProbeInfo.cascadeCount - 1; i >= 0 ; i--)
	  {
	    vkResetCommandBuffer(lightProbePipeline.commandBuffers[i], 0);

	    VkCommandBufferBeginInfo beginInfo{};
	    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	    beginInfo.flags = 0; // Optional
	    beginInfo.pInheritanceInfo = nullptr; // Optional

	    vkBeginCommandBuffer(lightProbePipeline.commandBuffers[i], &beginInfo);
	
	    vkCmdBindPipeline(lightProbePipeline.commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, lightProbePipeline.pipeline);
	
	    vkCmdBindDescriptorSets(lightProbePipeline.commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE,
				    lightProbePipeline.pipelineLayout,
				    0, 1,
				    &rayInputInfo.assemblerPool.descriptorSets[imageIndex][rayInputInfo.assemblerBuffer.indexIntoPool]
				    , 0, 0);
	    vkCmdBindDescriptorSets(lightProbePipeline.commandBuffers[i], VK_PIPELINE_BIND_POINT_COMPUTE,
				    lightProbePipeline.pipelineLayout,
				    1, 1,
				    &lightProbeInfo.computeUniformPool.descriptorSets[imageIndex][lightProbeInfo.computeUniforms[i].indexIntoPool]
				    , 0, 0);


	    //Cascade info cascade 0
	    vkCmdPushConstants(lightProbePipeline.commandBuffers[i],
			       lightProbePipeline.pipelineLayout,
			       VK_SHADER_STAGE_COMPUTE_BIT,
			       0, sizeof(CascadeInfo),
			       &cascadeInfos[i]);	

	    vkCmdDispatch(lightProbePipeline.commandBuffers[i],
			  lightProbeInfo.textures[i].width / 8,
			  lightProbeInfo.textures[i].height / 8,
			  lightProbeInfo.textures[i].depth / 8);
	
	    if (vkEndCommandBuffer(lightProbePipeline.commandBuffers[i]) != VK_SUCCESS) {
	      throw std::runtime_error("failed to record command buffer!");
	    }
	
	    //Syncronization info for compute 
	    VkSubmitInfo submitInfo{};
	    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	    VkSemaphore waitSemaphores[1];
	    VkSemaphore signalSemaphores[1];
	    if (i == lightProbeInfo.cascadeCount - 1)
	      {
		waitSemaphores[0] = {imageAvailableSemaphore}; //Wait for image availble, ie prev frame done
	      signalSemaphores[0] = {lightProbeInfo.semaphoreChain[i]}; //Signal to next chain ready
	      }
	    else if (i == 0)
	      {
		waitSemaphores[0] = {lightProbeInfo.semaphoreChain[i + 1]}; //Wait for previous cascade
		signalSemaphores[0] = {probeInfoFinishedSemaphore}; //Signal that cascades are done
	      }
	    else
	      {
	      waitSemaphores[0] = {lightProbeInfo.semaphoreChain[i + 1]}; //Wait previous cascade
	      signalSemaphores[0] = {lightProbeInfo.semaphoreChain[i]}; //Signal to next cascade
	      }
	    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT}; //Wait for this stage before writing to image

	    submitInfo.waitSemaphoreCount = 1;
	    submitInfo.pWaitSemaphores = waitSemaphores;
	    submitInfo.pWaitDstStageMask = waitStages;
	    submitInfo.commandBufferCount = 1;
	    submitInfo.pCommandBuffers = &lightProbePipeline.commandBuffers[i];
	    submitInfo.signalSemaphoreCount = 1;
	    submitInfo.pSignalSemaphores = signalSemaphores;
    

	    if (vkQueueSubmit(computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
	      throw std::runtime_error("failed to submit compute command buffer!");
	    };

	    lightProbeInfo.transitionImageToSampled(i);
	  }
      }

    

    //RASTERIZATION -----------------------------------------------------------------
    if (USE_RASTER)
      {
	vkResetCommandBuffer(drawCommandBuffer.commandBuffer, 0);
    
	drawCommandBuffer.begin(renderPass.renderPass,
			    swapChain.framebuffers[imageIndex],
			    swapChain.extent);

	
	vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipeline);
	
	//Cascade debug view info
	vkCmdPushConstants(drawCommandBuffer.commandBuffer,
			   graphicsPipeline.layout,
			   VK_SHADER_STAGE_FRAGMENT_BIT,
			   0, sizeof(CascadeInfo),
			   &cascadeInfos[viewCascade]);	
	vkCmdBindDescriptorSets(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				graphicsPipeline.layout,
				1, 1, &cameraUniformPool.descriptorSets[imageIndex][mainCamera.ubo.indexIntoPool], 0, nullptr);
	vkCmdBindDescriptorSets(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				graphicsPipeline.layout,
				2, 1, &lightProbeInfo.drawUniformPool.descriptorSets[imageIndex][lightProbeInfo.drawUniform.indexIntoPool], 0, nullptr);

	drawCommandBuffer.record(graphicsPipeline.pipeline,
			     graphicsPipeline.layout,
			     &rayInputInfo.vertexBuffer,
			     &matrixPool.descriptorSets[imageIndex][ratModel->modelUniform.indexIntoPool],
				 ratModel
			     );
	drawCommandBuffer.record(graphicsPipeline.pipeline,
			     graphicsPipeline.layout,
			     &rayInputInfo.vertexBuffer,
			     &matrixPool.descriptorSets[imageIndex][cornellScene->modelUniform.indexIntoPool],
				 cornellScene
			     );
	drawCommandBuffer.record(graphicsPipeline.pipeline,
				 graphicsPipeline.layout,
				 &rayInputInfo.vertexBuffer,
				 &matrixPool.descriptorSets[imageIndex][cornellScene->modelUniform.indexIntoPool],
				 cornellRightWall
				 );
	drawCommandBuffer.record(graphicsPipeline.pipeline,
			     graphicsPipeline.layout,
			     &rayInputInfo.vertexBuffer,
			     &matrixPool.descriptorSets[imageIndex][cornellScene->modelUniform.indexIntoPool],
				 cornellLeftWall
			     );

	drawCommandBuffer.record(graphicsPipeline.pipeline,
			     graphicsPipeline.layout,
			     &rayInputInfo.vertexBuffer,
			     &matrixPool.descriptorSets[imageIndex][cornellLight->modelUniform.indexIntoPool],
				 cornellLight
			     );
	drawCommandBuffer.record(graphicsPipeline.pipeline,
			     graphicsPipeline.layout,
			     &rayInputInfo.vertexBuffer,
			     &matrixPool.descriptorSets[imageIndex][cornellCeilLight->modelUniform.indexIntoPool],
				 cornellCeilLight
			     );
    


	// DEBUG LINES ---------------------------------------------------------------

	if (DRAW_DEBUG_LINES)
	  {
	    
	    vkCmdBindPipeline(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipeline.pipeline);
	    //Cascade debug view info
	    vkCmdPushConstants(drawCommandBuffer.commandBuffer,
			       linePipeline.layout,
			       VK_SHADER_STAGE_VERTEX_BIT,
			       0, sizeof(CascadeInfo),
			       &cascadeInfos[viewCascade]);	

	    vkCmdBindDescriptorSets(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				    linePipeline.layout,
				    1, 1, &cameraUniformPool.descriptorSets[imageIndex][mainCamera.ubo.indexIntoPool], 0, nullptr);
	    vkCmdBindDescriptorSets(drawCommandBuffer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				    linePipeline.layout,
				    2, 1, &lightProbeInfo.drawUniformPool.descriptorSets[imageIndex][lightProbeInfo.drawUniform.indexIntoPool], 0, nullptr);

	    drawCommandBuffer.record(linePipeline.pipeline,
				      linePipeline.layout,
				     &lightProbeInfo.lineVBO,
				     &matrixPool.descriptorSets[imageIndex][cornellScene->modelUniform.indexIntoPool],
				     0,lightProbeInfo.lineCount * 2
				      );
	    
	  }

	debugConsole.draw(drawCommandBuffer.commandBuffer);
	
	drawCommandBuffer.end();

	//Syncronization info for graphics 
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = {probeInfoFinishedSemaphore}; //Wait for this before submitting draw
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //Wait for this stage before writing to image
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore}; //Signal to this when drawing is done
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCommandBuffer.commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
    
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
	  throw std::runtime_error("failed to submit draw command buffer!");
	}



      }
    //RAY -----------------------------------------------------------------
    else {
      	vkResetCommandBuffer(rayPipeline.commandBuffer, 0);
	
	rayPipeline.transitionSwapChainForComputeWrite(rayBackBuffer.image, swapChain.images[imageIndex]);
	
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	vkBeginCommandBuffer(rayPipeline.commandBuffer, &beginInfo);
	
	vkCmdBindPipeline(rayPipeline.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, rayPipeline.pipeline);
	
	vkCmdBindDescriptorSets(rayPipeline.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
				rayPipeline.pipelineLayout,
				1, 1, &cameraUniformPool.descriptorSets[imageIndex][mainCamera.ubo.indexIntoPool], 0, nullptr);

	vkCmdBindDescriptorSets(rayPipeline.commandBuffer,
				VK_PIPELINE_BIND_POINT_COMPUTE, rayPipeline.pipelineLayout,
				0, 1,
				&rayInputInfo.assemblerPool.descriptorSets[imageIndex][rayInputInfo.assemblerBuffer.indexIntoPool]
				, 0, 0);
	//Cascade info cascade 0
	vkCmdPushConstants(rayPipeline.commandBuffer,
			   rayPipeline.pipelineLayout,
			   VK_SHADER_STAGE_COMPUTE_BIT,
			   0, sizeof(RayDebugPushConstant),
			   &rayDebugPushConstant);	
	
	vkCmdDispatch(rayPipeline.commandBuffer, swapChain.extent.width / 32, swapChain.extent.height / 32,  1); //switch to swapchain width and height

	
	if (vkEndCommandBuffer(rayPipeline.commandBuffer) != VK_SUCCESS) {
	  throw std::runtime_error("failed to record command buffer!");
	}
	
	//Syncronization info for compute 
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphore}; //Wait for this before submitting draw
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; //Wait for this stage before writing to image
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore}; //Signal to this when drawing is done
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &rayPipeline.commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
    

	if (vkQueueSubmit(computeQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
	  throw std::runtime_error("failed to submit compute command buffer!");
	};

	rayPipeline.transitionSwapChainForComputeTransfer(rayBackBuffer.image);
	rayPipeline.copyTextureToSwapChain(swapChain.images[imageIndex], rayBackBuffer.image,
					   swapChain.extent.width,  swapChain.extent.height);
	rayPipeline.transitionSwapChainForComputePresent(swapChain.images[imageIndex]);
    }

    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore}; //Signal to this when drawing is done
    presentInfo.pWaitSemaphores = signalSemaphores; //what semaphore to wait for before showing screen
    VkSwapchainKHR swapChains[] = {swapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(presentQueue, &presentInfo);

  }

  void processInputs()
  {
    
    auto fpsNow = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(fpsNow - fpsPrev).count();
    //printf("\r%.8f %4.2f", deltaTime, 1.0f / deltaTime);
    fpsPrev = std::chrono::high_resolution_clock::now();
    
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    if (cameraEnabled)
      {
	glm::vec2 mouseDiff = glm::vec2(mouseX, mouseY) - glm::vec2(xPos, yPos);

	double cameraSensitivity = 0.01f;
	mainCamera.direction = glm::rotate(mainCamera.direction,
					   (float)(mouseDiff.x * cameraSensitivity),
					   glm::vec3(0.0f, 1.0f, 0.0f));
	mainCamera.direction = glm::rotate(mainCamera.direction,
					   (float)(mouseDiff.y * cameraSensitivity),
					   glm::cross(mainCamera.direction,
						      glm::vec3(0.0f, 1.0f, 0.0f)));
      }
    mouseX = xPos; mouseY = yPos;


    if (inputInfo.keysPressed[GLFW_KEY_A])
      mainCamera.position -= glm::cross(mainCamera.direction * deltaTime * 3.0f,
					glm::vec3(0.0, 1.0, 0.0));
    if (inputInfo.keysPressed[GLFW_KEY_D])
      mainCamera.position += glm::cross(mainCamera.direction * deltaTime * 3.0f,
					glm::vec3(0.0, 1.0, 0.0));

    if (inputInfo.keysPressed[GLFW_KEY_W])
      mainCamera.position += mainCamera.direction * deltaTime * 3.0f;
    if (inputInfo.keysPressed[GLFW_KEY_S])
      mainCamera.position -= mainCamera.direction * deltaTime * 3.0f;

    if (inputInfo.keysPressed[GLFW_KEY_SPACE])
      mainCamera.position += glm::vec3(0.0,  deltaTime * 3.0f, 0.0);
    if (inputInfo.keysPressed[GLFW_KEY_LEFT_CONTROL])
      mainCamera.position += glm::vec3(0.0, -deltaTime * 3.0f, 0.0);

    
  }

  
  void mainLoop() {

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      
      processInputs();
      drawFrame();
      //      glfwSetWindowShouldClose(window, GL_TRUE);

    }

    if (vkDeviceWaitIdle(device) != VK_SUCCESS)
      {
	  throw std::runtime_error("failed to wait idle!!");	
      }

  }


  void cleanup() {
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, probeInfoFinishedSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);
	
    vkDestroyCommandPool(device, drawCommandPool, nullptr);
    vkDestroyCommandPool(device, transientCommandPool, nullptr);
    vkDestroyCommandPool(device, computeCommandPool, nullptr);
    
    for (auto framebuffer : swapChain.framebuffers) {
      vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto imageView : swapChain.imageViews) {
      vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroyPipeline(device, graphicsPipeline.pipeline, nullptr);	
    vkDestroyPipelineLayout(device, graphicsPipeline.layout, nullptr);

    vkDestroyPipeline(device, linePipeline.pipeline, nullptr);	
    vkDestroyPipelineLayout(device, linePipeline.layout, nullptr);

    debugConsole.destroy();
    rayInputInfo.destroy();
    vkDestroyRenderPass(device, renderPass.renderPass, nullptr);

    vkDestroySwapchainKHR(device, swapChain.swapChain, nullptr);

    rayPipeline.destroy();
    lightProbePipeline.destroy();
    delete ratModel;
    delete cornellScene;
    delete cornellLight;
    delete cornellRightWall;
    delete cornellLeftWall;
    delete cornellCeilLight;
    lightProbeInfo.destroy();
    mainCamera.ubo.destroy();
    cameraUniformPool.destroy();

    depthTexture.destroy();
    matrixPool.destroy();
    rayBackBuffer.destroy();
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    if (enableVulkanValidationLayers) {
      debugUtils.DestroyDebugUtilsMessengerEXT(instance, debugUtils.debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
    
    glfwDestroyWindow(window);
    glfwTerminate();
  }
    
};



int main()
{
  V2Engine app;

  try {
    app.run();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
  
}
  
