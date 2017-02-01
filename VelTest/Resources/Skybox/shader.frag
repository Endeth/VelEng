#version 450 core

in vec3 verUV;

out vec4 FragColor;

uniform samplerCube CubeSampler;

void main()
{
	FragColor = texture(CubeSampler, verUV);
}

