#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : enable

#include "brdf/lighting.glsl"

//Scene Descriptor
#include "common/cameraLayout0.glsl"

layout(set = 1, binding = 0) uniform sampler2D positionTexture;
layout(set = 1, binding = 1) uniform sampler2D colorTexture;
layout(set = 1, binding = 2) uniform sampler2D normalsTexture;
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessTexture;

layout(set = 2, binding = 0) uniform LightData
{
	vec4 ambient;
	vec4 sunlightDirection;
	vec4 sunlightColor;
	mat4 sunlightViewProj;
	int sunlightShadowMapID;

	int pointLightsCount;
	PointLightsBuffer pointLightsBuffer;
} lightData;
layout (set = 2, binding = 1) uniform texture2D shadowsTexture;
layout (set = 2, binding = 2) uniform sampler shadowsSampler;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

#include "brdf/shadows.glsl"

void main() 
{
	vec3 position = texture(positionTexture, inUV).rgb;
	vec4 colorSample = texture(colorTexture, inUV);
	vec3 color = colorSample.rgb;
	float alpha = colorSample.a; //Currently for skybox
	vec3 normal = texture(normalsTexture, inUV).rgb;
	vec3 metallicRoughness = texture(metallicRoughnessTexture, inUV).rgb;

	float roughnessFactor = sceneCamera.testData.g;
	float metallicFactor = 0.01f;
	float roughness = roughnessFactor * metallicRoughness.g;
	roughness = max(roughness, 1e-2);
    float metallic = metallicFactor * metallicRoughness.b;

	vec3 dielectricSpecular = vec3(0.04);
    vec3 black = vec3(0.0);
    vec3 diffuseColor = mix(color * (1.0 - dielectricSpecular.r), black, metallic);
    vec3 f0 = mix(dielectricSpecular, color, metallic);
	vec3 view = normalize(sceneCamera.cameraPosition.xyz - position);

	vec3 ambientColor = color * lightData.ambient.xyz;
	vec3 sunlightDir = normalize(lightData.sunlightDirection.xyz);
	vec3 sunlightColor = ProcessDirectionalLight(normal, view, diffuseColor, roughness, f0, sunlightDir, lightData.sunlightColor.xyz);
	vec3 pointLightsIntensity = ProcessPointLights(position, normal, view, diffuseColor, roughness, f0, lightData.pointLightsBuffer, lightData.pointLightsCount);

	vec3 projectionCoords = (lightData.sunlightViewProj * vec4(position, 1.0f)).xyz;
	projectionCoords.xy = projectionCoords.xy * vec2(0.5f, 0.5f) + vec2(0.5f, 0.5f);
	float bias = GetShadowBias(sunlightDir, normal);

	projectionCoords.z += bias; //bias;
	//float sampledShadow = texture(sampler2D(shadowsTexture, shadowsSampler), projectionCoords.xy).r;
	//sunlightColor *= projectionCoords.z > sampledShadow  ? 1.0 : 0.0;;
	float sampledShadow = MultiSampleShadowMapGrid(projectionCoords);
	sunlightColor *= sampledShadow;

	vec3 colorResult = max(ambientColor, sunlightColor) + pointLightsIntensity;
	colorResult.x = min(colorResult.x, color.x);
	colorResult.y = min(colorResult.y, color.y);
	colorResult.z = min(colorResult.z, color.z);

	//Skybox fuckery
	colorResult *= alpha;
	colorResult += color * (1 - alpha);

	outColor = vec4(colorResult, 1.0f);
	//sampledShadow = sampledShadow * 2;
	//outColor = vec4(sampledShadow, sampledShadow, sampledShadow, 1.0f);
}