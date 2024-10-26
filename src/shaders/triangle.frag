#version 450

layout(set=0, binding = 1) uniform sampler2D texSampler;
//layout(set=2, binding = 1) uniform sampler3D probeSampler;
layout(set = 2, binding = 0, rgba8) uniform readonly image3D probeInfo;
layout(set = 2, binding = 1, rgba8) uniform readonly image3D probeInfo2;	
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 worldNormal;
layout(location = 0) out vec4 outColor;

layout( push_constant ) uniform cascadeConstant {
  int cascade;
  int quadrant;
  int allQuadrants;
} cascadeInfo;


void main() {

  vec3 width = vec3(2.2);
  vec3 center = vec3(0.0,0.0,0.0);
  
  //need -1:1 -> 0:1
  vec3 probeTexLocation = (worldPos - center) / width;
  probeTexLocation += 0.5;
  //have 0-1
  //Now 0-1 to divided for cascades
//vec3(dirTilingCount, dirTilingCount, 1.0);
  //quadrantLocalCoord.z = 1.0;

  vec4 radiance = vec4(0.0);

    for (int cascade = cascadeInfo.cascade; cascade < cascadeInfo.cascade + 1; cascade++)
  //  for (int cascade = 0; cascade < 1; cascade++)
    {
  if (worldPos.x > width.x/2 ||
      worldPos.x <-width.x/2 ||
      worldPos.y > width.y/2 ||
      worldPos.y <-width.y/2 ||
      worldPos.z > width.z/2 ||
      worldPos.z <-width.z/2)
    {
      radiance = vec4(0.0,1.0,1.0,1.0);
      break;
    }
  

      vec3 normalizedCoords = probeTexLocation;
      //probeTexLocation.z = cascade;
      int dirCount = cascade == 0.0 ? 8 : 64; //TODO unhardcode
      int dirTilingCount = cascade == 0.0 ? 2 : 4; //TODO cube root

      int gridSize = 32 / int((cascade + 1)); 
      //int tilingCount = int(textureSize(probeSampler,0).x) / gridSize / dirTilingCount;
      //int tilingCount = int(imageSize(probeInfo).x) / gridSize / dirTilingCount;
  
      //Coord in relation to whole quadrant
      //vec2 gridCoord = vec2(int(normalizedCoords.y * tilingCount * tilingCount) % tilingCount,
      //                     int(normalizedCoords.y * tilingCount));
      //probeTexLocation.x = normalizedCoords.x / tilingCount + gridCoord.x / tilingCount;
      //probeTexLocation.y = normalizedCoords.z / tilingCount + gridCoord.y / tilingCount;
  
      //scale down to quadrant
      vec3 quadrantLocalCoord = probeTexLocation / vec3(dirTilingCount, dirTilingCount, dirTilingCount);

      if (probeTexLocation.x > 1.0 ||
	  probeTexLocation.x < 0.0 ||
	  probeTexLocation.y > 1.0 ||
	  probeTexLocation.y < 0.0 ||
	  probeTexLocation.z > 1.0 ||
	  probeTexLocation.z < 0.0
	  )
	{
	  radiance = vec4(1.0,0.0,1.0,1.0);
	  break;
	}
      ivec3 coord = ivec3(quadrantLocalCoord * imageSize(probeInfo));
      for (int dir = cascadeInfo.quadrant; dir < cascadeInfo.quadrant + 1; dir++)//dirCount; dir++)
      //for (int dir = 0; dir < 1; dir++)//dirCount; dir++)
        {
          
          ivec3 quadrantOffset = ivec3((dir * imageSize(probeInfo).x) % dirTilingCount,
                                       (dir * imageSize(probeInfo).y) / dirTilingCount,
                                       0);
	  ivec3 imgCoord = coord + quadrantOffset;
	  if (imgCoord.x > imageSize(probeInfo).x || imgCoord.x > imageSize(probeInfo).y)
	    radiance = vec4(1.0,0.0,1.0,1.0);
	  else
	    radiance = imageLoad(probeInfo, coord + quadrantOffset);
        }
    }
  radiance.a = 1.0;
  //outColor = texture(texSampler, fragTexCoord) * radiance + 0.1;
  outColor = radiance;

}
