#version 450

layout(set=0, binding = 1) uniform sampler2D texSampler;
layout(set=2, binding = 0) uniform sampler3D probeSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {

    outColor = texture(texSampler, fragTexCoord);
}