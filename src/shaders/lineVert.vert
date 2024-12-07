#version 450

/*layout(set=0, binding = 0) uniform UniformBufferObject {
    mat4 model;
} ubo;
*/
layout(set=1, binding = 1) uniform cameraUniform {
  mat4 view;
  mat4 proj;
  float nearClip;
  float farClip;
} _MainCamera;


layout( push_constant ) uniform cascadeConstant {
  int cascade;
  int quadrant;
  float start;
  float end;
} cascadeInfo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 0) out vec3 outColour;

void main() {

  gl_Position = _MainCamera.proj * _MainCamera.view * vec4(position, 1.0);
  outColour = colour;
  if (colour == vec3(0.0))
      gl_Position = vec4(-2.0,-2.0,-2.0,-2.0);

  //if (cascadeInfo.allQuadrants == 1) return;
  //cull if not in right direction debug
  int dirTilingCount = cascadeInfo.cascade == 0 ? 2 : 4;
  int gridSize = cascadeInfo.cascade == 0 ? 32 : 16;
  int lineIdx = gl_VertexIndex / 2;
  int imgSize = gridSize*dirTilingCount;
  //0 - imgSize**3 -> vec3(0-gridSize)

  //outColour = vec3(float(lineIdx) / (imgSize * imgSize*imgSize));
  return;
  int x = lineIdx % imgSize;
  lineIdx /= imgSize;
  int y = lineIdx % imgSize;
  lineIdx /= imgSize;
  int z = lineIdx;
  ivec3 invocationID = ivec3(x,y,z);
  outColour = vec3(invocationID) / imgSize;
}
