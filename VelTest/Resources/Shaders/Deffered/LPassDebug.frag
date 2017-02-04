#version 450 core

in vec2 verUV;

out vec4 FragColor;

uniform sampler2D gDiffSpec;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D depthMap;
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
uniform vec3 ambientLight;

float ShadowCalculation(vec3 fragPos)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - pLights[0].Position.xyz;
	
    // Use the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
	
    // It is currently in linear range between [0,1]. Let's re-transform it back to original depth value
    closestDepth *= 25;
	
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
	
    // Now test for shadows
    float bias = 0.05; // We use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    vec3 Diffuse = texture(gDiffSpec, verUV).rgb;
    float Specular = texture(gDiffSpec, verUV).a;
	vec3 FragPos = texture(gPosition, verUV).rgb;
	vec3 Normal = texture(gNormal, verUV).rgb; //already normalized
	float Depth = texture(gDepth, verUV).r;
	
	if(Specular < -0.5) // Skybox hack
	{
		FragColor = vec4(Diffuse.r, Diffuse.g, Diffuse.b, 1.0); 
		//FragColor = vec4(0,0,0,0); //Skybox off
	}
	else
	{
		if(renderMode == 0) // without shadows
		{
			//standard rendering
			vec3 viewDir  = normalize(viewPos - FragPos);
			vec3 lighting = Diffuse * ambientLight;
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
				float shadow = ShadowCalculation(FragPos); 
				lighting+= (diffuse + specular); 
			} 
			FragColor = vec4(lighting, 1.0);
		}
		if(renderMode == 666) // with shadows
		{
			//standard rendering
			vec3 viewDir  = normalize(viewPos - FragPos);
			vec3 lighting;
			vec3 ambient = Diffuse * ambientLight;
			for(int i = 0; i < NR_LIGHTS; ++i)
			{
				// Diffuse
				vec3 lightDir = normalize(pLights[i].Position - FragPos);
				vec3 diffuse = max(dot(Normal, lightDir), 0.0) * pLights[i].Color * Diffuse;
				// Specular

				vec3 halfwayDir = normalize(lightDir + viewDir);  
				float spec = pow(max(dot(Normal, halfwayDir), 0.0), 64.0);
				vec3 specular = spec *  pLights[i].Color * Specular;
				
				float distance = length(pLights[i].Position - FragPos);
				float attenuation = 1.0 / (1.0 + pLights[i].Linear * distance + pLights[i].Quadratic * distance * distance);
				diffuse *= attenuation;
				specular *= attenuation;
				// Calculate shadow
				float shadow = ShadowCalculation(FragPos);                  
				lighting = (diffuse + specular);  
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
		float closestDepth = texture(depthMap, FragPos.rgb).r;
		FragColor = vec4(Depth, Depth, Depth, 1.0);
	}
}

