#version 450 

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // fragment from emitvertex output

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; //specifies to which face is rendering
        for(int i = 0; i < 3; ++i) //each vertex
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  