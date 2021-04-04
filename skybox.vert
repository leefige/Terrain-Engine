/*
 * GLSL Vertex Shader for skybox.
 */

#version 460 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 mapCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    mapCoord = texCoord;
}