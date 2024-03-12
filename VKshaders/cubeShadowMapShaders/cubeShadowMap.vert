#version 450

// #extension GL_ARB_separate_shader_objects : enable
// #extension GL_ARB_shading_language_420pack : enable

#define CUBE_DEMENTIONS 6

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
	mat4 spaceMatrix;
	vec3 lightPosition;
	float farPlane;
	mat4 jointMatrices[18];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoordinate;
layout(location = 3) in vec4 inJointIndices;
layout(location = 4) in vec4 inWeights;

layout(location = 0) out vec4 outFragmentPosition;
layout(location = 1) out vec3 outLightPosition;
layout(location = 2) out float outFarPlane;

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
    outFragmentPosition = worldPosition;
//	outFragmentPosition = ubo.spaceMatrix * worldPosition;
	outLightPosition = ubo.lightPosition;
	outFarPlane = ubo.farPlane;

	gl_Position = ubo.spaceMatrix * worldPosition;
}


