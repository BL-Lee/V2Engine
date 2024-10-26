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


layout( push_constant ) uniform cascadeConstant {
  int cascade;
  int quadrant;
  int allQuadrants;
} cascadeInfo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 0) out vec3 outColour;

void main() {

  gl_Position = _MainCamera.proj * _MainCamera.view * ubo.model * vec4(position, 1.0);
  outColour = colour;


  //if (cascadeInfo.allQuadrants == 1) return;
  //cull if not in right direction debug
  int dirTilingCount = cascadeInfo.cascade == 0 ? 2 : 4;
  int gridSize = cascadeInfo.cascade == 0 ? 32 : 16;
  int lineIdx = gl_VertexIndex / 2;
  ivec3 invocationID = ivec3(
			     lineIdx / (512 * 512),
			     (lineIdx / (512)) % 512,
			     lineIdx % (512)
			     );
  ivec3 quadrant = ivec3(invocationID.x / (512 / dirTilingCount),
			 invocationID.y / (512 / dirTilingCount),
			 invocationID.z / (512 / dirTilingCount)
			 );
  int quadIdx = quadrant.x * dirTilingCount * dirTilingCount +
    quadrant.y * dirTilingCount +
    quadrant.z;
  //outColour = vec3(vec2(quadrant) / dirTilingCount,0.0);
  //outColour = vec3(gl_VertexIndex / 2000000.0);
  if (quadIdx != cascadeInfo.quadrant)// ||
    //        colour == vec3(0.3))
    gl_Position = vec4(-2.0,-2.0,-2.0,-2.0);

}
