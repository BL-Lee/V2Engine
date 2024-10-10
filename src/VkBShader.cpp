#include "VkBShader.hpp"

VkShaderModule VkBShader::createShaderFromFile(const std::string& filename)
  {
    return VkBShader::createShaderModule(VkBShader::readShader(filename));
  }
  
std::vector<char> VkBShader::readShader(const std::string& filename)
  {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }


VkShaderModule VkBShader::createShaderModule(const std::vector<char>& code)
  {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = (const uint32_t*)code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create shader module");
    }
    return shaderModule;
  }

