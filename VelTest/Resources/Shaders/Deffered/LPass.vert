#version 450 core
layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec2 vUV;

out vec2 verUV;

void main()
{
    gl_Position = vec4(vVertex, 1.0f);
    verUV = vUV;
}