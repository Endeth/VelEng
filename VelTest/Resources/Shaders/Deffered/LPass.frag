#version 450 core

in vec2 verUV;

out vec4 FragColor;

uniform sampler2D gDiffSpec;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform int renderMode;

struct PointLight {
    vec3 Position;
    vec3 Color;
    
	float Constant;
    float Linear;
    float Quadratic;
};

const int NR_LIGHTS = 2;
uniform PointLight pLights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{
    vec3 Diffuse = texture(gDiffSpec, verUV).rgb;
    float Specular = texture(gDiffSpec, verUV).a;
	vec3 FragPos = texture(gPosition, verUV).rgb;
	vec3 Normal = texture(gNormal, verUV).rgb;
	float Depth = texture(gDepth, verUV).r;
	
	if(Specular < -0.5)
	{
		FragColor = vec4(Diffuse.rgb, 1.0);
	}
	else
	{
		if(renderMode == 0)
		{
			//standard rendering
			vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
			vec3 viewDir  = normalize(viewPos - FragPos);
			
			for(int i = 0; i < NR_LIGHTS; ++i)
			{
				// Diffuse
				vec3 lightDir = normalize(pLights[i].Position - FragPos);
				vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pLights[i].Color;
				
				// Specular
				vec3 halfwayDir = normalize(lightDir + viewDir);  
				float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
				vec3 specular = pLights[i].Color * spec * Specular;
				
				// Attenuation
				float distance = length(pLights[i].Position - FragPos);
				float attenuation = 1.0 / (1.0 + pLights[i].Linear * distance + pLights[i].Quadratic * distance * distance);
				diffuse *= attenuation;
				specular *= attenuation;
				lighting += diffuse + specular;
			} 
			FragColor = vec4(lighting, 1.0);
		}
	}
	
	//DEBUG MODES
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

