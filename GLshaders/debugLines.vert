#version 410 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 colorData;

out vec3 color;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	color       = colorData;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
