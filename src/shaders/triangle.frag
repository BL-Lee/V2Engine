#version 450

layout(set=0, binding = 1) uniform sampler2D texSampler;

//layout(set=2, binding = 1) uniform sampler3D probeSampler;
layout(set = 2, binding = 0, rgba8) uniform restrict readonly image3D probeInfo[4];
//layout(set = 1, binding = 0, rgba8) uniform readonly image3D probeInfo[2];
//layout(set = 2, binding = 1, rgba8) uniform readonly image3D probeInfo2;	
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 worldNormal;
layout(location = 3) in vec3 TEMP_EYE_POS;
layout(location = 0) out vec4 outColor;

layout( push_constant ) uniform cascadeConstant {
  int cascade;
  int quadrant;
  float start;
  float end;
  int lineView;
} cascadeInfo;

float pi = 3.14159;
float phi = pi * (sqrt(5.0) - 1.0);
void main() {

  //outColor = texture(texSampler, fragTexCoord);
  //return;
  vec3 width = vec3(2.2);
  vec3 center = vec3(0.0,1.0,0.0);
  

  
  //need -1:1 -> 0:1
  vec3 probeTexLocation = (worldPos - center) / width;
  probeTexLocation += 0.5;
  //have 0-1
  //Now 0-1 to divided for cascades

  vec4 radiance = vec4(0.0);
  vec3 TEMP_CAM_LOC = vec3(5.0,1.0,0.0);
  vec3 incidentRay = normalize(worldPos - TEMP_EYE_POS);
  vec3 reflectedRay = normalize(incidentRay - 2*worldNormal*(dot(incidentRay, worldNormal)));
  //for (int cascade = cascadeInfo.cascade; cascade < cascadeInfo.cascade + 1; cascade++)
  //  for (int cascade = 0; cascade < 2; cascade++)
  for (int cascade = 0; cascade < 1; cascade++) //If we're accumulating down the cascade chain.. only need to sample lowest
    {
      //Debug show outside of range
      vec3 pos = worldPos - center;
      if (pos.x > width.x/2 ||
	  pos.x <-width.x/2 ||
	  pos.y > width.y/2 ||
	  pos.y <-width.y/2 ||
	  pos.z > width.z/2 ||
	  pos.z <-width.z/2)
	{
	  radiance = vec4(0.0,1.0,1.0,1.0);
	  break;
	}
  

      vec3 normalizedCoords = probeTexLocation;
      int dirCount = int(pow(8, cascade + 1)); //TODO unhardcode
      int dirTilingCount = int(pow(2, cascade + 1)); //TODO cube root
      
      //scale down to quadrant 
      vec3 quadrantLocalCoord = probeTexLocation / vec3(dirTilingCount, dirTilingCount, dirTilingCount);

      ivec3 coord = ivec3(quadrantLocalCoord * imageSize(probeInfo[cascade]));
      //for (int dir = cascadeInfo.quadrant; dir < cascadeInfo.quadrant + 1; dir++)//dirCount; dir++)
      for (int dir = 0; dir < dirCount; dir++)//dirCount; dir++)
        {
	  float y = 1.0 - (dir / float(dirCount)) * 2.0;
	  float radius = sqrt(1.0 - y * y);

	  float theta = phi * dir;
	  float x = cos(theta) * radius;
	  float z = sin(theta) * radius;
	  vec3 rayDir = vec3(x,y,z);
	  //	  if (dot(reflectedRay, worldNormal) < 0.8)
	  //	    continue;

          ivec3 quadrantOffset = ivec3(dir % dirTilingCount,
				       (dir / dirTilingCount) % dirTilingCount,
				       dir / (dirTilingCount * dirTilingCount)) * imageSize(probeInfo[cascade]) / dirTilingCount;
	  ivec3 imgCoord = coord + quadrantOffset;
	  if (imgCoord.x > imageSize(probeInfo[cascade]).x || imgCoord.x > imageSize(probeInfo[cascade]).y)
	    radiance = vec4(1.0,0.0,1.0,1.0);
	  else
	    radiance += imageLoad(probeInfo[cascade], coord + quadrantOffset) / (dirCount / 8.0) ;
        }
    }

  radiance.a = 1.0;
  outColor = radiance + 0.1;
  //outColor = radiance;

}
