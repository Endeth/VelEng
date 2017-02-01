#version 450 core

layout(location = 0) in vec3 vVertex;

out vec3 verUV;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	verUV = vVertex;
	mat4 rotationMat = mat4(mat3(V));
	mat4 MV = rotationMat*M;
	mat4 MVP = P*MV;
	vec4 pos = MVP*vec4(vVertex,1);
	gl_Position = pos.xyww;
}