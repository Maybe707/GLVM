#version 410 core

void main()
{             
	gl_FragDepth = gl_FragCoord.z;
}
