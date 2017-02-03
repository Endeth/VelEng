#version 450 core

in vec3 verUV;

layout (location = 0) out vec4 gDiffSpec;

uniform samplerCube CubeSampler;

void main()
{
	gDiffSpec = vec4(texture(CubeSampler, verUV).xyz, -1);
}

