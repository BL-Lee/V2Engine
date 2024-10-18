#version 450

layout(set=0, binding = 1) uniform sampler2D texSampler;
layout(set=2, binding = 1) uniform sampler3D probeSampler;
layout(set = 2, binding = 0, rgba8) uniform readonly image3D probeInfo;	
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 0) out vec4 outColor;

vec3 getQuadrantLocalCoord(vec3 coord) //normalized world Coord to local quadrant coord
{
  int dirCount = coord.z == 0.0 ? 4 : 16; //TODO unhardcode
  int dirTilingCount = int(sqrt(dirCount)); // #quadrants on each axis

  int gridSize = 64 / int((coord.z + 1)); 
  int tilingCount = 8 / int((coord.z + 1));

  ivec3 localCoord = ivec3(coord * textureSize(probeSampler,0) / dirTilingCount / tilingCount); //pixel coords
  return vec3(localCoord) / gridSize;
}

void main() {
  vec3 width = vec3(2.2);
  vec3 center = vec3(0.0,1.0,0.0);
  vec3 texWidth = textureSize(probeSampler,0);

  //need -1:1 -> 0:1
  vec3 probeTexLocation = (worldPos - center) / width;
  probeTexLocation += 0.5;
  //have 0-1
  //Now 0-1 to divided for cascades
  vec3 normalizedCoords = probeTexLocation;
  probeTexLocation.z = 1.0;

  int dirCount = probeTexLocation.z == 0.0 ? 4 : 16; //TODO unhardcode
  int dirTilingCount = int(sqrt(dirCount)); // #quadrants on each axis

  int gridSize = 64 / int((probeTexLocation.z + 1)); 
  int tilingCount = int(textureSize(probeSampler,0).x) / gridSize / dirTilingCount;
  
  //Coord in relation to whole quadrant
  // vec2(int(normalizedCoords.y * 32) % 8, int(normalizedCoords.y * 8))
  // (4,7)
  // (0.9 / 8 + 4 / 8, 0.2 / 8 + 7/8)
  vec2 gridCoord = vec2(int(normalizedCoords.y * tilingCount * tilingCount) % tilingCount,
			int(normalizedCoords.y * tilingCount));
  probeTexLocation.x = normalizedCoords.x / tilingCount + gridCoord.x / tilingCount;
  probeTexLocation.y = normalizedCoords.z / tilingCount + gridCoord.y / tilingCount;
  
  //scale down to quadrant
  vec3 quadrantLocalCoord = probeTexLocation / vec3(2.0,2.0, 1.0);//vec3(dirTilingCount, dirTilingCount, 1.0);
  //quadrantLocalCoord.z = 1.0;
  outColor = texture(texSampler, fragTexCoord) *
    //    vec4((texture(probeSampler, quadrantLocalCoord).rgb + 0.1),1.0);
    vec4((texture(probeSampler, vec3(quadrantLocalCoord.xy,1.0)).rgb),1.0);
    //vec4((imageLoad(probeInfo, ivec3(quadrantLocalCoord.xy,1.0)).rgb),1.0);

  //outColor = vec4(probeTexLocation.xy - gridCoord * tiling, 0.0,1.0);
}
