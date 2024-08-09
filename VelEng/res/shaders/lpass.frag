#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require

#include "brdf/lighting.glsl"

//Scene Descriptor
#include "common/cameraLayout0.glsl"

layout(set = 1, binding = 0) uniform sampler2D positionTexture;
layout(set = 1, binding = 1) uniform sampler2D colorTexture;
layout(set = 1, binding = 2) uniform sampler2D normalsTexture;
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessTexture;

layout(set = 2, binding = 0) uniform ShadowlessLightData
{
	vec4 ambient;
	vec4 sunlightDirection;
	vec4 sunlightColor;

	int pointLightsCount;
	PointLightsBuffer pointLightsBuffer;
} lightData;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
	vec3 position = texture(positionTexture, inUV).xyz;
	vec3 color = texture(colorTexture, inUV).xyz;
	vec3 normal = texture(normalsTexture, inUV).xyz;
	vec3 metallicRoughness = texture(metallicRoughnessTexture, inUV).xyz;


	float roughnessFactor = sceneCamera.testData.g;
	float metallicFactor = sceneCamera.testData.b;
	float roughness = roughnessFactor * metallicRoughness.g;
	roughness = max(roughness, 1e-2);
    float metallic = metallicFactor * metallicRoughness.b;

	vec3 dielectricSpecular = vec3(0.04);
    vec3 black = vec3(0.0);
    vec3 diffuseColor = mix(color * (1.0 - dielectricSpecular.r), black, metallic);
    vec3 f0 = mix(dielectricSpecular, color, metallic);
	vec3 view = normalize(sceneCamera.cameraPosition.xyz - position);

	vec3 ambientColor = color * lightData.ambient.xyz;
	vec3 sunlightColor = ProcessDirectionalLight(normal, view, diffuseColor, roughness, f0, lightData.sunlightDirection.xyz, lightData.sunlightColor.xyz);
	vec3 pointLightsIntensity = ProcessPointLights(position, normal, view, diffuseColor, roughness, f0, lightData.pointLightsBuffer, lightData.pointLightsCount);

	vec3 colorResult = ambientColor + sunlightColor + pointLightsIntensity;
	colorResult.x = min(colorResult.x, color.x);
	colorResult.y = min(colorResult.y, color.y);
	colorResult.z = min(colorResult.z, color.z);

	outColor = vec4(colorResult, 1.0f);
}