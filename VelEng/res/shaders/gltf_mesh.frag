#version 460

//Scene Descriptor
layout(set = 0, binding = 0) uniform SceneData
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	vec4 ambientColor;
	vec4 sunlightDirection; //w used as intensity
	vec4 sunlightColor;
} sceneData;

//Material Descriptor
layout(set = 1, binding = 0) uniform GLTFMaterialData
{
	vec4 colorFactors;
	vec4 metallicFactors;
} materialData;

layout(set = 1, binding = 1) uniform sampler2D colorTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicTexture;

//Input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

//Output
layout (location = 0) out vec4 outFragColor;

void main() 
{
	float minimumLight = 0.1f;
	float lightValue = max(dot(inNormal, sceneData.sunlightDirection.xyz), minimumLight);

	vec3 color = inColor * texture(colorTexture, inUV).xyz;
	vec3 ambient = color * sceneData.ambientColor.xyz;

	outFragColor = vec4(color * lightValue * sceneData.sunlightDirection.w + ambient, 1.0f);
	outFragColor = vec4(inNormal, 1.0f);
}