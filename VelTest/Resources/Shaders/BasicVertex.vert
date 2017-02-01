#version 450 core

layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

uniform float Time;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 verFragPos;
out vec3 verNormal;
out vec2 verUV;


void main()
{
	mat4 MV = V*M;
	mat4 rotationMat = mat4(mat3(MV));
	mat4 MVP = P*MV;
	
	gl_Position = MVP*vec4(vVertex,1);

	//verSmoothColor *= sin(Time * 4 + (vVertex.x) * 2) * 0.5;
	
	verFragPos = vec3(M * vec4(vVertex, 1.0f));
    verNormal = mat3(transpose(inverse(M))) * vNormal; 
	verUV = vUV;
}