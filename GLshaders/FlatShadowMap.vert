#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoordinates;
layout (location = 3) in vec4 jointIndices;
layout (location = 4) in vec4 weights;

uniform mat4 jointMatrices[18];

uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

void main()
{
	mat4 skinMatrix;
	if (int(jointIndices.x) != -1) {
		skinMatrix =
			weights.x * jointMatrices[int(jointIndices.x)] +
			weights.y * jointMatrices[int(jointIndices.y)] +
			weights.z * jointMatrices[int(jointIndices.z)] +
			weights.w * jointMatrices[int(jointIndices.w)];
	} else {
		skinMatrix = mat4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
			);
	}

	vec4 worldPosition = modelMatrix * skinMatrix * vec4(aPosition, 1.0);
	
	gl_Position = lightSpaceMatrix * worldPosition;
//	gl_Position = lightSpaceMatrix * modelMatrix * vec4(aPosition, 1.0);
}
