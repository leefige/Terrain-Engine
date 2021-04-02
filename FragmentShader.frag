/*
 * GLSL Fragment Shader code for OpenGL version 4.6
 */

#version 460 core

in vec3 ourColor;

out vec4 color;

void main()
{
	color = vec4(ourColor, 1.0f);
}