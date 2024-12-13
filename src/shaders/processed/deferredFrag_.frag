#version 450

layout (binding = 1) uniform sampler2D samplerColor;
layout (binding = 2) uniform sampler2D samplerNormalMap;



layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 normal;
layout(location = 3) in float matIndex;

layout (location = 0) out vec4 outNormal;
layout (location = 1) out vec4 outWorldPos;
//layout (location = 2) out vec4 outUV;

void main() 
{
	outWorldPos = vec4(worldPos, matIndex);

	outNormal = vec4(normal.xy, 1.0, 1.0);

//	outUV = vec4(fragTexCoord,0.5,1.0);
}
