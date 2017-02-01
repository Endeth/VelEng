#version 450 core

smooth in vec4 verSmoothColor;
in vec2 verUV;

out vec4 FragColor;

uniform sampler2D TextureSampler;

void main()
{
	FragColor = verSmoothColor;
	FragColor += texture(TextureSampler, verUV) /2;
	//FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

