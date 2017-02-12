#version 450 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far plane
    lightDistance = lightDistance / farPlane;
    
    gl_FragDepth = lightDistance;
}  

