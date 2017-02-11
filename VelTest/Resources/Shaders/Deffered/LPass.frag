#version 450 core

in vec2 verUV;
out vec4 FragColor;

struct DirectionalLight
{
	vec3 Direction;
	vec3 ColorDiff;
	vec3 ColorSpec;
	mat4 lightSpaceMatrix;
};

struct PointLight 
{
    vec3 Position;
    vec3 ColorDiff;
	vec3 ColorSpec;
    mat4 lightSpaceMatrix;
	float Constant;
    float Linear;
    float Quadratic;
};

struct SpotLight //not implemented yet
{
	vec3 Position;
	vec3 Direction;
	vec3 ColorDiff;
	vec3 ColorSpec;
	mat4 lightSpaceMatrix;
	float CutOff;
	float OuterCutOff;
};

uniform sampler2D gDiffSpec; //GBUFFER DATA
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDepth;

const int NR_LIGHTS = 16;
uniform DirectionalLight dirLight;

uniform int PointLightsCount;
uniform PointLight pointLights[NR_LIGHTS];

//uniform int SpotLightsCount;
//uniform SpotLight spotLights[NR_LIGHTS];

uniform vec3 viewPos;

uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;
uniform sampler2D shadowMap3;
uniform sampler2D dirLightShadowMap;

uniform vec3 ambientLight;

float ShadowCalcDepth(vec3 projCoordinates, vec2 offset, int ite);
float ShadowCalculation(int lIte);

vec3 CalcDirLight(vec3 viewDir);
vec3 CalcPointLights(vec3 viedDir);

vec3 Diffuse = texture(gDiffSpec, verUV).rgb;
float Specular = texture(gDiffSpec, verUV).a;
vec3 FragPos = texture(gPosition, verUV).rgb;
vec3 Normal = texture(gNormal, verUV).rgb;
float Depth = texture(gDepth, verUV).r;

void main()
{
	
	if(Specular < -0.5)
	{
		FragColor = vec4(Diffuse.rgb, 1.0);
	}
	else
	{
		vec3 lighting  = Diffuse * ambientLight; // hard-coded ambient component
		vec3 viewDir  = normalize(viewPos - FragPos);
		lighting += CalcDirLight(viewDir);
		lighting += CalcPointLights(viewDir);
		FragColor = vec4(lighting, 1.0);
	}
}

vec3 CalcDirLight(vec3 viewDir)
{
	vec3 lightDir = normalize(dirLight.Direction);
	vec3 diffuse = max(dot(Normal, -lightDir), 0.0) * Diffuse * dirLight.ColorDiff;
	
	vec3 reflectDir = reflect(lightDir, Normal); 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
	vec3 specular = dirLight.ColorSpec * spec * Specular;
	
	vec4 fragPosLightSpace = dirLight.lightSpaceMatrix * vec4(FragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float currentDepth = projCoords.z;
	
    float bias = max(0.05 * (1.0 - dot(Normal, -lightDir)), 0.005);
	float shadow = 0.0;
    
	//PCF
	vec2 texelSize = 1.0 / textureSize(dirLightShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth =  texture(dirLightShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
	shadow /= 9.0;
	//float closestDepth = texture(dirLightShadowMap, projCoords.xy).r; 
	//shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
	if(projCoords.z > 1.0)
        shadow = 0.0;
			
			
	return ((diffuse + specular) *(1.0 - shadow));
}

vec3 CalcPointLights(vec3 viewDir)
{
	vec3 lighting;
	lighting = vec3(0,0,0);
	for(int lightIte = 0; lightIte < NR_LIGHTS; ++lightIte)
	{
		// Diffuse
		vec3 lightDir = normalize(pointLights[lightIte].Position - FragPos);
		vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pointLights[lightIte].ColorDiff;
		
		// Specular
		vec3 halfwayDir = normalize(lightDir + viewDir);  
		float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
		vec3 specular = pointLights[lightIte].ColorSpec * spec * Specular;
		
		// Attenuation
		float distance = length(pointLights[lightIte].Position - FragPos);
		float attenuation = 1.0 / (1.0 + pointLights[lightIte].Linear * distance + pointLights[lightIte].Quadratic * distance * distance);
		diffuse *= attenuation;
		specular *= attenuation;
		if(lightIte < 4) //Shadows only for the first 4 lights 
		{
			float shadow = ShadowCalculation(lightIte); //Choosing right shadowmap
			lighting += ((diffuse + specular) *(1.0 - shadow));
		}
		else //No shadow calculations for rest of lights
		{
			lighting += (diffuse + specular);
		}
	}
	return lighting;
}

float ShadowCalcDepth(vec3 projCoordinates, vec2 offset, int ite) //returns depth for
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

float ShadowCalculation(int lIte)
{
	vec4 fragPosLightSpace = pointLights[lIte].lightSpaceMatrix * vec4(FragPos, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	float currentDepth = projCoords.z;
	
    vec3 lightDir = normalize(pointLights[lIte].Position - FragPos);
    float bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);
	float shadow = 0.0;
    
	//PCF
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth =  ShadowCalcDepth(projCoords, vec2(x, y), lIte); // no need to recalc texelSize each time
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
	
	if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}