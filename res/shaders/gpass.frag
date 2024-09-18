#version 460

#extension GL_GOOGLE_include_directive : require

//Scene Descriptor
#include "common/cameraLayout0.glsl"

layout(set = 1, binding = 0) uniform sampler2D colorTexture;
layout(set = 1, binding = 1) uniform sampler2D normalsTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
/*layout(set = 1, binding = 3) uniform PBRData
{
	vec4 color;
	vec4 metallicRoughnessFactor;

	vec4 extra[14];
} pbrData;*/


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV;
layout (location = 2) in mat3 inTBN;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outColor;
layout (location = 2) out vec4 outNormals;
layout (location = 3) out vec4 outMetallicRoughness;

void main() 
{
	vec3 color = texture(colorTexture, inUV).xyz;

	outPosition = vec4(inPosition, gl_FragCoord.z);
	outColor = vec4(color, 1.0f);
	vec3 signedNormals = normalize(texture(normalsTexture, inUV).xyz * 2.0 - 1.0);
	outNormals = vec4(normalize(inTBN * signedNormals), 0.0f);
	outMetallicRoughness = texture(metallicRoughnessTexture, inUV);
}