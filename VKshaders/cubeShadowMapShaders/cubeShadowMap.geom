#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(location = 3) uniform UniformBufferLightSpace {
	mat4 shadowMatrices[6];
};
										 
layout(location = 4) out vec4 fragmentPosition; ///< FragmentPosition from GS (output per emitvertex)

void main()
{
	for(int face = 0; face < 6; ++face)
    {
		gl_Layer = face; ///< Built-in variable that specifies to which face we render.
		for(int i = 0; i < 3; ++i)
		{
			fragmentPosition = gl_in[i].gl_Position;
			gl_Position = shadowMatrices[face] * fragmentPosition;
			EmitVertex();
		}
		EndPrimitive();
	}
}
															
