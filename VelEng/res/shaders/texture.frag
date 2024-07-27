#version 460

layout(set = 0, binding = 0) uniform sampler2D displayTexture;

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

//output write
layout (location = 0) out vec4 outFragColor;

void main() 
{
	//return red
	outFragColor = texture(displayTexture, inUV);
}