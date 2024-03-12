#version 330 core
out vec4 FragColor;

in vec2 TextureCoords;

uniform sampler2D depthMap;
uniform float nearPlane;
uniform float farPlane;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));	
}

void main()
{             
    float depthValue = texture(depthMap, TextureCoords).r;
//    FragColor = vec4(vec3(LinearizeDepth(depthValue) / farPlane), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
