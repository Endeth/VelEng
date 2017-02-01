#version 450 core

in vec3 verFragPos;
in vec3 verNormal;
in vec2 verUV;

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct Light {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;
uniform Light light;
uniform Material material;
uniform float Time;

void main()
{
	vec3 diffuseSample = (texture(material.diffuse, verUV).xyz);
	vec3 ambient = light.ambient * diffuseSample;
	
	vec3 norm = normalize(verNormal);
	
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * diffuseSample;
	
	vec3 viewDir = normalize(viewPos - verFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * (texture(material.specular, verUV).xyz));
	
	vec3 result = ambient + diffuse + specular;
	//vec3 result = vec3(verUV.x, 0, verUV.y);
	
	FragColor = vec4(result,1);
}

