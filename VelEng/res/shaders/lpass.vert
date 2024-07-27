#version 460

#extension GL_EXT_buffer_reference : require

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

//Push constant
struct Vertex
{
	vec3 position;
	float u;
	vec3 normal;
	float v;
	vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
	Vertex vertices[];
};

layout(push_constant) uniform constants
{
	mat4 model;
	VertexBuffer vertexBuffer;
} PushConstants;

//Output
layout (location = 0) out vec2 outUV;
//layout (location = 0) out vec3 outPosition;
//layout (location = 1) out vec3 outColor;
//layout (location = 2) out vec3 outNormal;
//layout (location = 3) out vec2 outUV;

void main()
{
	Vertex vert = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	vec4 position = vec4(vert.position, 1.0f);

	gl_Position = PushConstants.model * position;

	//outPosition = (PushConstants.model * position).xyz;
	//outNormal = (PushConstants.model * vec4(vert.normal, 0.f)).xyz;
	//outColor = vert.color.xyz;
	outUV.x = vert.u;
	outUV.y = vert.v;
}