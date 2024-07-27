#version 460

//Scene Descriptor
layout(set = 0, binding = 0) uniform SceneCamera
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
} sceneCamera;

layout(set = 1, binding = 0) uniform sampler2D colorTexture;
layout(set = 1, binding = 1) uniform sampler2D normalsTexture;
layout(set = 1, binding = 2) uniform sampler2D specularTexture;


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outNormals;

void main() 
{
	vec3 color = inColor * texture(colorTexture, inUV).xyz;
	//vec3 ambient = color * sceneData.ambientColor.xyz;

	outPosition = vec4(inPosition, gl_FragCoord.z);
	outColor = color;
	outNormals = inNormal;
}