#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common/vertex.glsl"

layout(set = 0, binding = 0) uniform LightData
{
	mat4 viewProj;
} lightData;

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
	Vertex vertices[];
};

layout(push_constant) uniform constants
{
	mat4 model;
	VertexBuffer vertexBuffer;
} PushConstants;

void main()
{
	Vertex vert = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	vec4 position = vec4(vert.position, 1.0f);
	gl_Position = lightData.viewProj * PushConstants.model * position;
}
