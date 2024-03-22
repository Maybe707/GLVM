#version 450

#extension GL_EXT_debug_printf : enable

#define SPOT_LIGHT_SPACE_MATRIX_CONTAINER_SIZE 2
#define DIRECTIONAL_LIGHT_SPACE_MATRIX_CONTAINER_SIZE 2

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 jointMatrices[18];

	vec3      ambient;
    float     shininess;

	mat4 spotSpaceMatrix[SPOT_LIGHT_SPACE_MATRIX_CONTAINER_SIZE];
	int spotLightsNumber;
	
	mat4 dirSpaceMatrix[DIRECTIONAL_LIGHT_SPACE_MATRIX_CONTAINER_SIZE];
	int directionalLightsNumber;
} ubo;

layout(location = 5) out VS_OUT {
	vec3 fragmentPosition;
	vec3 normal;
	vec2 textureCoords;
	vec4 fragmentPositionDirectionalLightSpace[DIRECTIONAL_LIGHT_SPACE_MATRIX_CONTAINER_SIZE];
	vec4 fragmentPositionSpotLightSpace[SPOT_LIGHT_SPACE_MATRIX_CONTAINER_SIZE];

	vec3      ambient;
    float     shininess;
} vs_out;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoordinate;
layout(location = 3) in vec4 inJointIndices;
layout(location = 4) in vec4 inWeights;

layout(location = 0) out vec3 outFragmentPosition;
layout(location = 1) out vec3 outFragmentNormal;
layout(location = 2) out vec2 outFragmentTextureCoordinate;

// layout(set = 1, binding = 0) uniform TestDirLightSpaceMatrixUBO {
// 	mat4 spotSpaceMatrix[SPOT_LIGHT_SPACE_MATRIX_CONTAINER_SIZE];
// 	int spotLightsNumber;
	
// 	mat4 dirSpaceMatrix[DIRECTIONAL_LIGHT_SPACE_MATRIX_CONTAINER_SIZE];
// 	int directionalLightsNumber;
// } spaceMat;

void main() {
	mat4 skinMatrix;
	if (int(inJointIndices.x) != -1) {
		skinMatrix =
			inWeights.x * ubo.jointMatrices[int(inJointIndices.x)] +
			inWeights.y * ubo.jointMatrices[int(inJointIndices.y)] +
			inWeights.z * ubo.jointMatrices[int(inJointIndices.z)] +
			inWeights.w * ubo.jointMatrices[int(inJointIndices.w)];
	} else {
		skinMatrix = mat4(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
			);
	}

	vec4 worldPosition = ubo.model * skinMatrix * vec4(inPosition, 1.0);
	
	vs_out.fragmentPosition = worldPosition.xyz;
//	vs_out.normal = transpose(inverse(mat3(ubo.model))) * vec3(skinMatrix * vec4(inNormal, 1.0));
	vs_out.normal = mat3(transpose(inverse(ubo.model * skinMatrix))) * inNormal;
	vs_out.textureCoords = inTextureCoordinate;
	for (int i = 0; i < ubo.directionalLightsNumber; ++i) 
		vs_out.fragmentPositionDirectionalLightSpace[i] = ubo.dirSpaceMatrix[i] * worldPosition;
	
	for (int i = 0; i < ubo.spotLightsNumber; ++i) 
		vs_out.fragmentPositionSpotLightSpace[i] = ubo.spotSpaceMatrix[i] * worldPosition;

	vs_out.ambient = ubo.ambient;
	vs_out.shininess = ubo.shininess;
	
    gl_Position = ubo.proj * ubo.view * worldPosition;
//	gl_Position = worldPosition * ubo.model * ubo.view * ubo.proj;
	outFragmentPosition = worldPosition.xyz;
    outFragmentNormal = inNormal;
    outFragmentTextureCoordinate = inTextureCoordinate;
}
