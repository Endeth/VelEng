#version 450 core
layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 lightSpaceMatrix;


void main()
{
    gl_Position = lightSpaceMatrix * M * vec4(vVertex, 1.0f);
}