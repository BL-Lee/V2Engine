#version 450

layout(set=0, binding = 1) uniform sampler2D texSampler;
layout(set=2, binding = 1) uniform sampler3D probeSampler;
layout(set = 2, binding = 0, rgba8) uniform readonly image3D probeInfo;	
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 worldNormal;
layout(location = 0) out vec4 outColor;


void main() {

  vec3 width = vec3(2.2);
  vec3 center = vec3(0.0,1.0,0.0);
  
  //need -1:1 -> 0:1
  vec3 probeTexLocation = (worldPos - center) / width;
  probeTexLocation += 0.5;
  //have 0-1
  //Now 0-1 to divided for cascades
//vec3(dirTilingCount, dirTilingCount, 1.0);
  //quadrantLocalCoord.z = 1.0;
  vec3 dirs[16] = {

    normalize(vec3(0.0, 1.0, 0.0)),
    normalize(vec3(0.0, -1.0, 0.0)),
    normalize(vec3(0.0, 0.0, 1.0)),
    normalize(vec3(0.0, 0.0, -1.0)),
    
    normalize(vec3(1.0, 0.0, 0.0)),
    normalize(vec3(-1.0, 0.0, 0.0)),

    normalize(vec3(-1.0, -1.0, -1.0)),
    normalize(vec3(1.0, -1.0, -1.0)),
    normalize(vec3(-1.0, 1.0, -1.0)),
    normalize(vec3(1.0, 1.0, -1.0)),

    normalize(vec3(-1.0, -1.0, 1.0)),
    normalize(vec3(1.0, -1.0, 1.0)),
    normalize(vec3(-1.0, 1.0, 1.0)),
    normalize(vec3(1.0, 1.0, 1.0)),
    normalize(vec3(-1.0, 0.0, 1.0)),
    normalize(vec3(1.0, 0.0, 1.0))

    
  };

  vec4 radiance = vec4(0.0);
  for (int cascade = 1; cascade < 2; cascade++)
    {

      vec3 normalizedCoords = probeTexLocation;
      probeTexLocation.z = cascade;
      int dirCount = cascade == 0.0 ? 4 : 16; //TODO unhardcode
      int dirTilingCount = int(sqrt(dirCount)); // #quadrants on each axis

      int gridSize = 64 / int((cascade + 1)); 
      //int tilingCount = int(textureSize(probeSampler,0).x) / gridSize / dirTilingCount;
      int tilingCount = int(imageSize(probeInfo).x) / gridSize / dirTilingCount;
  
      //Coord in relation to whole quadrant
      vec2 gridCoord = vec2(int(normalizedCoords.y * tilingCount * tilingCount) % tilingCount,
                            int(normalizedCoords.y * tilingCount));
      probeTexLocation.x = normalizedCoords.x / tilingCount + gridCoord.x / tilingCount;
      probeTexLocation.y = normalizedCoords.z / tilingCount + gridCoord.y / tilingCount;
  
      //scale down to quadrant
      vec3 quadrantLocalCoord = probeTexLocation / vec3(dirTilingCount, dirTilingCount, 1.0);

      
      ivec3 coord = ivec3(ivec3(quadrantLocalCoord.xy * imageSize(probeInfo).xy, cascade));
      for (int dir = 0; dir < 1; dir++)//dirCount; dir++)
        {
          
          ivec3 quadrantOffset = ivec3(dir % gridSize,
                                       dir / gridSize,
                                       0);
          radiance = imageLoad(probeInfo, coord);
        }
    }
  radiance.a = 1.0;
  outColor = texture(texSampler, fragTexCoord) * radiance;
  outColor = radiance;

}
