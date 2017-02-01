#version 450 core

layout(location = 0) in vec3 vVertex;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vUV;

smooth out vec4 verSmoothColor;
out vec2 verUV;

uniform float Time;
uniform mat4 MVP;

void main()
{
   verSmoothColor = vColor;
   verUV = vUV;
   //verSmoothColor *= sin(Time * 0.1 + (gl_Position.x + gl_Position.y + gl_Position.z));
   gl_Position =  MVP * vec4(vVertex,1);
  
}