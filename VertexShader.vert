/*
 * GLSL Vertex Shader code for OpenGL version 4.6
 */

#version 460 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 ourColor;

void main()
{
	gl_Position = vec4(position, 1.0);
	ourColor = color;
}
