#version 450 core

layout(location = 0) in vec3 vVertex;

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 P;

out vec3 verUV;

void main()
{
	verUV = vVertex;
	mat4 rotationMat = mat4(mat3(V));
	mat4 MV = rotationMat*M;
	mat4 MVP = P*MV;
	vec4 pos = MVP*vec4(vVertex,1);
	gl_Position = pos.xyww;
}