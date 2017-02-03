#version 450 core

layout (location = 0) in vec3 vVertex;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;

out vec2 verUV;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(vVertex, 1.0);
}  