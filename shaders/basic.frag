#version 450

#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

// shader input
// location 0 connects to the vertex shader
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

// output write
layout (location = 0) out vec4 outFragColor;

void main()  {	
	float lightValue = max(dot(inNormal, vec3(0.3f,1.f,0.3f)), 0.1f);

    // outFragColor = vec4(inColor * lightValue * sceneData.sunlightColor.w + sceneData.ambientColor.rgb, 1.0f);

    outFragColor = vec4(inColor * lightValue * sceneData.sunlightColor.w + sceneData.ambientColor.rgb, 1.0f);
}
