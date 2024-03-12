#version 410 core
in vec4 fragmentPosition;

uniform vec3 lightPosition;
uniform float farPlane;

void main()
{
	// Get distance between fragment and light source
	float lightDistance = length(fragmentPosition.xyz - lightPosition);

	// Map to [0;1] range by dividing by farPlane
	lightDistance = lightDistance / farPlane;

	// Write this as modified depth
	gl_FragDepth = lightDistance;
}
