#pragma once
#include "VkBGlobals.hpp"
#include <iostream>
#include <vector>
#include <fstream>
class VkBShader {

public:

  static VkShaderModule createShaderFromFile(const std::string& filename);
  static std::vector<char> readShader(const std::string& filename);
  static VkShaderModule createShaderModule(const std::vector<char>& code);

};
