#version 460

layout(set = 0, binding = 0) uniform samplerCube skyboxTexture;

layout(push_constant) uniform constants
{
	mat4 inverseViewProj;
	vec4 cameraPosition;
} PushConstants;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
    vec2 ndc = inUV * 2.0 - vec2(1.0);

    vec4 coord = PushConstants.inverseViewProj * vec4(ndc, 1.0, 1.0);
    vec3 samplePoint = normalize(coord.xyz / vec3(coord.w) - PushConstants.cameraPosition.xyz);

    outColor = vec4(texture(skyboxTexture, samplePoint).xyz, 1.0f);
    //outColor = vec4(inUV.x, inUV.y, 0.0f, 1.0f);
}