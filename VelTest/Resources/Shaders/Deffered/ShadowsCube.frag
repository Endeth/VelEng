#version 450 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
	    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by farPlane - 25 hard coded
    lightDistance = lightDistance / farPlane;
    
    gl_FragDepth = 1.0;
}

