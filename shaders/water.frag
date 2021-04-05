/*
 * GLSL Fragment Shader for ocean.
 */

#version 460 core

in vec2 mapCoord;
uniform sampler2D tex2D;
uniform float waterAlpha;

// output color
out vec4 color;

void main()
{
	color = vec4(vec3(texture2D(tex2D, mapCoord)), waterAlpha);
}
