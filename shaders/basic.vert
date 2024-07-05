#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common.glsl"

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
}; 

// if we were in C: const VertexBuffer* vertices;
// we are declaring a readonly buffer reference (pointer) to the vertex buffer
// buffer_reference in the layout is what makes this buffer a pointer
layout(buffer_reference, std430) readonly buffer VertexBuffer { 
	Vertex vertices[];
};

// push constant block
layout( push_constant ) uniform constants
{	
	mat4 render_matrix;
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	// load vertex data from device adress 
	// it's like PushConstants.vertexBuffer->vertices[gl_VertexIndex]
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = PushConstants.render_matrix * vec4(v.position, 1.0f);
	outColor = v.color.xyz;
	outNormal = vec3(0.5f, 0.5f, 0.f);
	outUV.x = v.uv_x; // we are just passing the uv coordinates to the fragment shader
	outUV.y = v.uv_y;
}
