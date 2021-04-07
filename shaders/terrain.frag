/*
 * GLSL Fragment Shader for terrain.
 */

#version 460 core

in vec2 mapCoord;
in float yPos;

// output color
out vec4 color;

uniform sampler2D texColor;
uniform sampler2D texDetail;

uniform float detailScale;

uniform float upY;

void main()
{
	// this part is under the visiable plane
	if (upY * yPos < 0) {
		discard;
	}

	vec4 myColor = texture2D(texColor, mapCoord);
	vec4 myDetail = texture2D(texDetail, detailScale * mapCoord);
	// GL_ADD_SIGNED: a + b - 0.5
	color = myColor + myDetail - 0.5f;
}
