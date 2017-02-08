#version 450 core

in vec2 verUV;

out vec4 FragColor;

uniform sampler2D gDiffSpec;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDepth;

struct PointLight {
    vec3 Position;
    vec3 Color;
    mat4 lightSpaceMatrix;
	float Constant;
    float Linear;
    float Quadratic;
};

const int NR_LIGHTS = 16;
uniform PointLight pLights[NR_LIGHTS];
uniform vec3 viewPos;
uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;
uniform sampler2D shadowMap3;
uniform vec3 ambientLight;

float ShadowCalcDepth(vec3 projCoordinates, vec2 offset, int ite)
{
	vec2 texelSize;
	if(ite == 0)
	{
		texelSize = 1.0 / textureSize(shadowMap0, 0);
		return texture(shadowMap0, projCoordinates.xy + offset * texelSize).r;
	}
	if(ite == 1)
	{
		texelSize = 1.0 / textureSize(shadowMap1, 0);
		return texture(shadowMap1, projCoordinates.xy + offset * texelSize).r;
	}
	if(ite == 2)
	{
		texelSize = 1.0 / textureSize(shadowMap2, 0);
		return texture(shadowMap2, projCoordinates.xy + offset * texelSize).r;
	}
	if(ite == 3)
	{
		texelSize = 1.0 / textureSize(shadowMap3, 0);
		return texture(shadowMap3, projCoordinates.xy + offset * texelSize).r;
	}
}

float ShadowCalculation(vec3 fragPos, vec3 normal, int lIte)
{
	vec4 fragPosLightSpace = pLights[lIte].lightSpaceMatrix * vec4(fragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float currentDepth = projCoords.z;
	
    vec3 lightDir = normalize(pLights[lIte].Position - fragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = 0.0;
    
	//PCF
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth =  ShadowCalcDepth(projCoords, vec2(x, y), lIte);
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
	
	if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

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
		vec3 lighting  = Diffuse * ambientLight; // hard-coded ambient component
		vec3 viewDir  = normalize(viewPos - FragPos);
		
		for(int lightIte = 0; lightIte < NR_LIGHTS; ++lightIte)
		{
			// Diffuse
			vec3 lightDir = normalize(pLights[lightIte].Position - FragPos);
			vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pLights[lightIte].Color;
			
			// Specular
			vec3 halfwayDir = normalize(lightDir + viewDir);  
			float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
			vec3 specular = pLights[lightIte].Color * spec * Specular;
			
			// Attenuation
			float distance = length(pLights[lightIte].Position - FragPos);
			float attenuation = 1.0 / (1.0 + pLights[lightIte].Linear * distance + pLights[lightIte].Quadratic * distance * distance);
			diffuse *= attenuation;
			specular *= attenuation;
			if(lightIte < 4) //Shadows only for the first 4 lights 
			{
				float shadow = ShadowCalculation(FragPos, Normal, lightIte);
				lighting += ((diffuse + specular) *(1.0 - shadow));
			}
			else //No shadow calculations for rest of lights
			{
				lighting += (diffuse + specular);
			}
		} 
		FragColor = vec4(lighting, 1.0);
		//FragColor = vec4(Diffuse.r,Diffuse.r,Diffuse.r, 1.0);
	}
}