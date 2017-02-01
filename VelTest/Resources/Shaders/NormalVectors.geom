#version 450 core

layout (points) in;
in vec4 verSmoothColor[];
in vec2 verUV[];

layout (triangle_strip, max_vertices=128) out;
out vec4 geoSmoothColor;
out vec2 geoUV;

uniform uint SubDivisions;
uniform mat4 MVP;

void main()
{
	if(false)
	{
		geoSmoothColor = verSmoothColor[0];
		geoUV = verUV[0];

		vec4 v0 = gl_in[0].gl_Position;
		vec4 v1 = gl_in[1].gl_Position;
		vec4 v2 = gl_in[2].gl_Position;
		
		float dx = abs(v0.x-v2.x)/SubDivisions;
		float dy = abs(v0.y-v1.y)/SubDivisions;
		
		float x = v0.x;
		float y = v0.y;
		
		
		gl_Position = MVP * (gl_in[0].gl_Position + vec4(4.0, 0.0, 0.0, 1.0));
		EmitVertex();
		geoSmoothColor = verSmoothColor[1];
		gl_Position = MVP * (gl_in[1].gl_Position + vec4(4.0, 0.0, 0.0, 1.0));
		EmitVertex();
		geoSmoothColor = verSmoothColor[2];
		gl_Position = MVP * (gl_in[2].gl_Position + vec4(4.0, 0.0, 0.0, 1.0));
		EmitVertex();
		EndPrimitive();
		
		
		for(int i = 0; i <SubDivisions * SubDivisions; i++)
		{
			gl_Position = MVP * vec4(x, y, 0, 1);
			EmitVertex();
			gl_Position = MVP * vec4(x, y + dy, 0, 1);
			EmitVertex();
			gl_Position = MVP * vec4(x + dx, y, 0, 1);
			EmitVertex();
			gl_Position = MVP * vec4(x + dx, y + dy, 0, 1);
			EmitVertex();
			EndPrimitive();
			
			x+=dx;
			
			if((i +1) % SubDivisions == 0)
			{
				x = v0.x;
				y += dy;
			}
		}
	}
}
