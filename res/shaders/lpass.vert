#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common/vertex.glsl"

//Scene Descriptor
#include "common/cameraLayout0.glsl"

//layout(set = 1, binding = 0) uniform sampler2D positionTexture;
//layout(set = 1, binding = 1) uniform sampler2D colorTexture;
//layout(set = 1, binding = 2) uniform sampler2D normalsTexture;
//layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessTexture;

//Push constant
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

void main()
{
	Vertex vert = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	vec4 position = vec4(vert.position, 1.0f);

	gl_Position = PushConstants.model * position;
	outUV.x = vert.u;
	outUV.y = vert.v;
}