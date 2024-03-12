#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;

out vec2 TextureCoords;

out VS_OUT {
	vec3 FragmentPosition;
	vec3 Normal;
	vec2 TextureCoords;
} vs_out;

//uniform mat4 aRotate_Matrix;
uniform mat4 aModel_Matrix;
uniform mat4 aView_Matrix;
uniform mat4 aProjection_Matrix;

uniform bool reverse_normals;

void main()
{
	vs_out.FragmentPosition           = vec3(aModel_Matrix * vec4(aPosition, 1.0));
	if(reverse_normals) // a slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
        vs_out.Normal = transpose(inverse(mat3(aModel_Matrix))) * (-1.0 * aNormal);
    else
        vs_out.Normal = transpose(inverse(mat3(aModel_Matrix))) * aNormal;
//	vs_out.Normal                     = aNormal;
	vs_out.TextureCoords              = aTextureCoords;
	gl_Position                       = aProjection_Matrix * aView_Matrix * aModel_Matrix * vec4(aPosition, 1.0);
}
