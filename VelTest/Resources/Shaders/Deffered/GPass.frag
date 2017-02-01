#version 450 core

in vec3 verVertex;
in vec3 verNormal;
in vec2 verUV;

layout(location = 0) out vec4 gDiffSpec;
layout(location = 1) out vec3 gPosition;
layout(location = 2) out vec3 gNormal;


uniform sampler2D diffuse;
uniform sampler2D specular;

void main()
{
	gDiffSpec.rgb = texture(diffuse, verUV).rgb;
	gDiffSpec.a = texture(specular, verUV).r;
	gNormal = normalize(verNormal);
	gPosition = verVertex;
	
}

