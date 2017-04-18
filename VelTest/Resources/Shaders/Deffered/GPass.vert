#version 450 core

layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 P;

out vec3 verVertex;
out vec3 verNormal;
out vec2 verUV;

void main()
{

	vec4 worldPos = M * vec4(vVertex, 1.0f);
	verVertex = worldPos.xyz;

	mat4 MVP = P*V*M;
	
	gl_Position = MVP*vec4(vVertex,1);

    verNormal = mat3(transpose(inverse(M))) * vNormal; 
	verUV = vUV;
}