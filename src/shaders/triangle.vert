#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  float nearClip;
  float farClip;
} _MainCamera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = _MainCamera.proj * _MainCamera.view * ubo.model * vec4(position, 1.0);
    fragColor = colour;
    fragTexCoord = texCoord;
}
