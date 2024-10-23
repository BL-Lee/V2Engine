#version 450

layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(set=1, binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  float nearClip;
  float farClip;
} _MainCamera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 0) out vec3 outColour;

void main() {
    gl_Position = _MainCamera.proj * _MainCamera.view * ubo.model * vec4(position, 1.0);
    outColour = colour;
}
