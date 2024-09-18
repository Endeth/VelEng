#ifndef LIGHTNING_GLSL
#define LIGHTNING_GLSL

#include "brdf/brdf.glsl"

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout(buffer_reference, std430) readonly buffer PointLightsBuffer
{
	PointLight lights[];
};

vec3 ProcessDirectionalLight(vec3 normal, vec3 view,
							 vec3 diffuseColor, float roughness, vec3 f0,
							 vec3 lightDirection, vec3 lightColor)
{
	//float intensity = dot(normal, lightDirection);
	//vec3 outColor = diffuseColor * lightColor * max(intensity, 0.0f);
	//return outColor;

	return BRDF(diffuseColor, roughness, f0, normal, view, lightDirection);
}

vec3 ProcessPointLights(vec3 position, vec3 normal, vec3 view,
						vec3 diffuseColor, float roughness, vec3 f0,
						PointLightsBuffer pointLightsBuffer, int lightsCount)
{
	vec3 pointLightsIntensity = vec3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < lightsCount; ++i)
	{
		PointLight pointLight = pointLightsBuffer.lights[i];
		float lightDistance = distance(pointLight.position.xyz, position) + 0.001f;
		float distanceFall = pointLight.color.a / (lightDistance * lightDistance);

		vec3 pointLightDirection = normalize(pointLight.position.xyz - position);
		//float lightIntensity = dot(normal, pointLightDirection) * distanceFall;
		//pointLightsIntensity += color.xyz * pointLight.color.xyz * max(lightIntensity, 0.0f);

		vec3 brdfLight = BRDF(diffuseColor, roughness, f0, normal, view, pointLightDirection);
		pointLightsIntensity += brdfLight * pointLight.color.xyz * distanceFall;
	}

	return pointLightsIntensity;
}

#endif