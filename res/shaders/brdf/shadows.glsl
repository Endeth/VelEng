#ifndef SHADOWS_SAMPLING_GLSL
#define SHADOWS_SAMPLING_GLSL

vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ),
   vec2( 0.94558609, -0.76890725 ),
   vec2( -0.094184101, -0.92938870 ),
   vec2( 0.34495938, 0.29387760 ),
   vec2( -0.91588581, 0.45771432 ),
   vec2( -0.81544232, -0.87912464 ),
   vec2( -0.38277543, 0.27676845 ),
   vec2( 0.97484398, 0.75648379 ),
   vec2( 0.44323325, -0.97511554 ),
   vec2( 0.53742981, -0.47373420 ),
   vec2( -0.26496911, -0.41893023 ),
   vec2( 0.79197514, 0.19090188 ),
   vec2( -0.24188840, 0.99706507 ),
   vec2( -0.81409955, 0.91437590 ),
   vec2( 0.19984126, 0.78641367 ),
   vec2( 0.14383161, -0.14100790 )
);

// Returns a pseudo random number based on a vec3 and an int.
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

#define DISK_RADIUS 5000.f
#define NUM_ELEMENTS_DISK 16
vec2 getPoissonDiskCoord(vec2 projCoords, int i) {
    int index = int(float(NUM_ELEMENTS_DISK)*random(projCoords.xyy, i))%NUM_ELEMENTS_DISK;
    return projCoords.xy + poissonDisk[index]/DISK_RADIUS;
}

float MultiSampleShadowMapPoisson(vec3 projCoords)
{
	float shadow = 0.f;
    int numSamples = 8;
    for (int i = 0; i < numSamples; ++i)
    {
        vec2 coord = getPoissonDiskCoord(projCoords.xy, i);
        shadow += (projCoords.z > texture(sampler2D(shadowsTexture, shadowsSampler), coord).r) ? 1.0f : 0.0f;
    }
    shadow /= float(numSamples);
    return shadow;
}

float MultiSampleShadowMapGrid(vec3 projCoords)
{
	float shadow = 0.f;
    
    vec2 texelSize = vec2(0.00005f, 0.00005f);//textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(sampler2D(shadowsTexture, shadowsSampler), projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += projCoords.z > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0f;
    return shadow;
}

float GetShadowBias(vec3 lightDirection, vec3 fragNormal)
{    
	float texelsSampleDistance = 0.0007; //0.001
	return ( texelsSampleDistance / 2 ) * (dot(lightDirection, fragNormal) + 1); //direction = Frag position - light position
}

#endif
