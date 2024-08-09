#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common/vertex.glsl"

//Scene Descriptor
#include "common/cameraLayout0.glsl"

layout(set = 1, binding = 0) uniform sampler2D colorTexture;
layout(set = 1, binding = 1) uniform sampler2D normalsTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;

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
layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec2 outUV;
layout (location = 2) out mat3 outTBN;


void main()
{
	Vertex vert = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	vec4 position = vec4(vert.position, 1.0f);
	gl_Position = sceneCamera.viewProj * PushConstants.model * position;
	outPosition = (PushConstants.model * position).xyz;

	outUV = vec2(vert.u, vert.v);

	vec3 T = normalize(vec3(PushConstants.model * vert.tangent));
	vec3 N = normalize((PushConstants.model * vec4(vert.normal, 0.f)).xyz);
	vec3 B = cross(N, T) * vert.tangent.w;
	outTBN = mat3(T, B, N);
}