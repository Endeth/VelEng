#version 460

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require

//Scene Descriptor
layout(set = 0, binding = 0) uniform SceneCamera
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
} sceneCamera;

layout(set = 1, binding = 0) uniform sampler2D positionTexture;
layout(set = 1, binding = 1) uniform sampler2D colorTexture;
layout(set = 1, binding = 2) uniform sampler2D normalsTexture;

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout(buffer_reference, std430) readonly buffer PointLightsBuffer
{
	PointLight lights[];
};

struct Lights
{
	vec4 ambient;
	vec4 sunlightDirection;
	vec4 sunlightColor;

	int pointLightsCount;
	PointLightsBuffer pointLightsBuffer;
};

layout(set = 2, binding = 0) uniform LightData
{
	Lights lights;
} lightData;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
	vec3 position = texture(positionTexture, inUV).xyz;
	vec3 color = texture(colorTexture, inUV).xyz;
	vec3 normals = texture(normalsTexture, inUV).xyz;
	vec3 ambientColor = color * lightData.lights.ambient.xyz;

	float sunlightIntensity = dot(normals, lightData.lights.sunlightDirection.xyz);
	vec3 sunlightColor = color.xyz * lightData.lights.sunlightColor.xyz * max(sunlightIntensity, 0.0f);

	vec3 pointLightsIntensity = vec3(0.0f, 0.0f, 0.0f);
	vec3 pointLightDirection;
	for(int i = 0; i < lightData.lights.pointLightsCount; ++i)
	{
		PointLight pointLight = lightData.lights.pointLightsBuffer.lights[i];
		float lightDistance = distance(pointLight.position.xyz, position) + 0.001f;
		float distanceFall = pointLight.color.a / (lightDistance * lightDistance);

		vec3 pointLightDirection = normalize(pointLight.position.xyz - position);
		float lightIntensity = dot(normals, pointLightDirection) * distanceFall;
		pointLightsIntensity += color.xyz * pointLight.color.xyz * max(lightIntensity, 0.0f);
	}

	vec3 colorResult = ambientColor + sunlightColor + pointLightsIntensity;
	colorResult.x = min(colorResult.x, color.x);
	colorResult.y = min(colorResult.y, color.y);
	colorResult.z = min(colorResult.z, color.z);

	//vec3 ambient = color * sceneData.ambientColor.xyz;
	outColor = vec4(colorResult, 1.0f);
	//outColor = vec4(inUV, 1.0f, 1.0f);
}