#version 450 core

in vec2 verUV;

out vec4 FragColor;

uniform sampler2D gDiffSpec;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform int renderMode;

void main()
{
    vec3 Diffuse = texture(gDiffSpec, verUV).rgb;
    float Specular = texture(gDiffSpec, verUV).a;
    vec3 FragPos = texture(gPosition, verUV).rgb;
    vec3 Normal = texture(gNormal, verUV).rgb;
	float Depth = texture(gDepth, verUV).r;

	if(renderMode == 0)
	{
		//standard rendering, diffuse for now
		FragColor = vec4(Diffuse.rgb, 1.0);
	}
	if(renderMode == 1)
	{
		//diffuse rendering
		FragColor = vec4(Diffuse.rgb, 1.0);
	}
	if(renderMode == 2)
	{
		//specular rendering
		FragColor = vec4(Specular, Specular, Specular,1.0);
	}
	if(renderMode == 3)
	{
		//position rendering
		FragColor = vec4(FragPos.rgb, 1.0);
	}
	if(renderMode == 4)
	{
		//normals rendering
		FragColor = vec4(Normal.rgb, 1.0);
	}
	if(renderMode == 5)
	{
		//depth rendering
		FragColor = vec4(Depth, Depth, Depth, 1.0);
	}
}

