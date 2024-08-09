#define PI 3.141592

// Specular BRDF functions
// SpecularBRDF() = D(...)*G(...)*F(...) / 4(n * v)(n * l) 

// D - GGX microfacets distribution NDF
float SpecularDistribution(float NoH, float roughness)
{
	float a = NoH * roughness;
	float k = roughness / (1.0 - NoH * NoH + a * a);
	return k * k * (1.0 / PI );
}

// G - Geometric shadowing
float GeometricShadowing(float NoV, float NoL, float roughness)
{
	float r2 = roughness * roughness;
	float XV = NoL * sqrt(NoV * NoV * (1.0 - r2) + r2);
	float XL = NoV * sqrt(NoL * NoL * (1.0 - r2) + r2);

	return 0.5 / (XV + XL);
}

float GeometricShadowingFast(float NoV, float NoL, float roughness)
{
	float XV = NoL * (NoV * (1.0 - roughness) + roughness);
	float XL = NoV * (NoL * (1.0 - roughness) + roughness);

	return 0.5 / (XV + XL);
}

// F
vec3 SpecularFresnelEffect(float u, vec3 f0)
{
	return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

// Diffuse BRDF functions

float LambertianDiffuse()
{
	return 1.0f / PI;
}

/*vec3 DiffuseFresnelEffect(float u, vec3 f0, float f90)
{
	return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float BRDFDiffuse(float NoV, float NoL, float LoH, float roughness)
{
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = DiffuseFresnelEffect(NoL, 1.0, f90);
    float viewScatter = DiffuseFresnelEffect(NoV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / PI);
}*/

vec3 BRDF(vec3 diffuseColor, float roughness, vec3 fresnel0, vec3 normal, vec3 view, vec3 light)
{
	vec3 halfAngle = normalize(view + light);

    float normalLightDot = clamp(dot(normal, light), 0.0, 1.0);
	float normalViewDot = abs(dot(normal, view)) + 1e-5;
    float normalHalfAngleDot = clamp(dot(normal, halfAngle), 0.0, 1.0);
    float lightHalfAngleDot = clamp(dot(light, halfAngle), 0.0, 1.0);

	//float roughness = perceptualRoughness * perceptualRoughness;

	float D = SpecularDistribution(normalHalfAngleDot, roughness);
	float V = GeometricShadowing(normalViewDot, normalLightDot, roughness);
	vec3 F = SpecularFresnelEffect(lightHalfAngleDot, fresnel0);
	vec3 specular = (D * V) * F;

	//vec3 diffuse = diffuseColor * BRDFDiffuse(normalViewDot, normalLightDot, lightHalfAngleDot, roughness);
	vec3 diffuse = diffuseColor * LambertianDiffuse();

	return specular + diffuse;
}

