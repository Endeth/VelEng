#version 460

#extension GL_GOOGLE_include_directive : require

//Scene Descriptor

layout(set = 1, binding = 0) uniform sampler2D colorTexture;
layout(set = 1, binding = 1) uniform sampler2D normalsTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 1, binding = 3) uniform PBRData
{
	vec4 color;
	vec4 metallicRoughnessFactor;

	vec4 extra[14];
} pbrData;


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV;
layout (location = 2) in mat3 inTBN;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outNormals;
layout (location = 3) out vec4 outMetallicRoughness;

void main() 
{
}