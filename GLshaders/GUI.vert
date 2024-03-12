#version 410 core
layout (location = 0) in vec3 vertexPosition;

uniform mat4 modelMatrix;
//uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
