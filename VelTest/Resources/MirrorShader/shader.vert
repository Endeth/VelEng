#version 450 core
  
layout(location=0) in vec3 aVertex;
layout(location=1) in vec2 aUV;

smooth out vec2 vUV;

uniform mat4 MVP;

void main()
{ 	 
	gl_Position = MVP * vec4(aVertex.xyz,1);
	vUV = aUV;
}