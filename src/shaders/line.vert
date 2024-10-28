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
  int imgSize = 32*dirTilingCount;
  //0 - imgSize**3 -> vec3(0-gridSize)

  //outColour = vec3(float(lineIdx) / (imgSize * imgSize*imgSize));
  return;
  int x = lineIdx % imgSize;
  lineIdx /= imgSize;
  int y = lineIdx % imgSize;
  lineIdx /= imgSize;
  int z = lineIdx;
  
  ivec3 invocationID = ivec3(x,y,z);
  
  /*ivec3 invocationID = ivec3(
                             lineIdx % (imgSize),
			     lineIdx / (imgSize * imgSize),
			     (lineIdx / (imgSize)) % imgSize

			     );
  */
  //0-gridSize -> 0-dirTilingCount
  outColour = vec3(invocationID) / imgSize;
    
    /*  ivec3 quadrant = ivec3(vec3(invocationID) / gridSize * dirTilingCount);
  int quadIdx = quadrant.x * dirTilingCount * dirTilingCount +
    quadrant.y * dirTilingCount +
    quadrant.z;
  outColour = vec3(quadrant) / dirTilingCount;
  //outColour = vec3(gl_VertexIndex / 2000000.0);
  if (quadIdx != cascadeInfo.quadrant)// ||
    //        colour == vec3(0.3))
    gl_Position = vec4(-2.0,-2.0,-2.0,-2.0);
    */
}
