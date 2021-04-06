/*
 * GLSL Vertex Shader for terrain.
 */

#version 460 core

// input vertex attributes
layout (location = 0) in vec3 position;

out vec2 mapCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    mapCoord = vec2(position.x, 1.0f - position.z);
}