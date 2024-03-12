#version 450

layout(location = 0) in vec4 inFragmentPosition;
layout(location = 1) in	vec3 inLightPosition;
layout(location = 2) in float inFarPlane;

void main()
{             
	float lightDistance = length(inFragmentPosition - vec4(inLightPosition, 1.0));
//	float lightDistance = length(inFragmentPosition);

	lightDistance = lightDistance / inFarPlane;

	gl_FragDepth = lightDistance;
}
