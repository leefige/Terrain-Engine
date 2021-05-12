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

uniform float time;

void main()
{
    //float newY = position.y + 0.5 * cos(time);
    vec4 worldPos = projection * view * model * vec4(position, 1.0f);
    worldPos.y += abs(0.4 * (cos(time * 1.7) + 1));
    gl_Position = worldPos;
    // texture resolution: 10.0f
    vec2 scaledCoord = 10.0f * texCoord;
    mapCoord = vec2(scaledCoord.x + xShift, scaledCoord.y + yShift);
    
    FragPos = vec3(model * vec4(position, 1.0f));
    Normal = mat3(transpose(inverse(model))) * inNormal;  
}