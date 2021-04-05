/*
 * GLSL Fragment Shader for terrain.
 */

#version 460 core

in vec2 mapCoord;
uniform sampler2D tex2D;

// output color
out vec4 color;

void main()
{
	color = texture2D(tex2D, mapCoord);
}
