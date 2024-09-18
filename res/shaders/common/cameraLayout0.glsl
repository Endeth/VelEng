#ifndef CAMERA_GLSL
#define CAMERA_GLSL

layout(set = 0, binding = 0) uniform SceneCamera
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 invViewProj;
	vec4 cameraPosition;
	vec4 testData;
} sceneCamera;

#endif