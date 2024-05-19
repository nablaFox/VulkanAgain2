#version 450

// shader input
// location 0 connects to the vertex shader
layout (location = 0) in vec3 inColor;

// output write
layout (location = 0) out vec4 outFragColor;

void main()  {
	outFragColor = vec4(inColor,1.0f);
}
