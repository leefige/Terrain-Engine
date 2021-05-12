/*
 * GLSL Vertex Shader for ocean.
 */

#version 460 core

// input vertex attributes
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 mapCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float xShift;
uniform float yShift;

uniform vec3 inNormal;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    // texture resolution: 10.0f
    vec2 scaledCoord = 10.0f * texCoord;
    mapCoord = vec2(scaledCoord.x + xShift, scaledCoord.y + yShift);
    
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * inNormal;  
}