#version 450

layout (binding = 1) uniform sampler2D samplerColor;
layout (binding = 2) uniform sampler2D samplerNormalMap;



layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 normal;
layout(location = 3) flat in uint matIndex;

layout (location = 0) out vec4 outNormal;
layout (location = 1) out vec4 outAlbedo;
//layout (location = 2) out vec4 outUV;
//layout todo material index,?

void main() 
{
  vec4 albedo = vec4(0.0);
  if (matIndex == 0) //diffuseGrey
    albedo = vec4(1.0,1.0,1.0,1.0);
  else if (matIndex == 1) //diffuseGrey
    albedo = vec4(0.5,0.5,0.5,1.0) * 0.2;
  else if (matIndex == 2) //diffuseGrey
    albedo = vec4(1.0,0.0,0.0,1.0) * 0.2;
  else if (matIndex == 3) //diffuseGrey
    albedo = vec4(0.0,1.0,0.0,1.0) * 0.2;
  //if (matIndex == 4) //reflective
  else
    albedo = vec4(1.0,1.0,0.0,1.0);
  
  albedo *= clamp(dot(normal, vec3(0.0,1.0,0.0)) + 0.8, 0, 1);
  outAlbedo = vec4(albedo.rgb, matIndex);

  float reflective = abs(matIndex - 4.0) < 0.01 ? 1.0 : 0.0;
  outNormal = vec4(normal.xyz, reflective);

//	outUV = vec4(fragTexCoord,0.5,1.0);
}
