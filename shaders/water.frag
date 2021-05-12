/*
 * GLSL Fragment Shader for ocean.
 */

#version 460 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 mapCoord;
in vec3 FragPos;  
in vec3 Normal;  

// output color
out vec4 color;

uniform sampler2D tex2D;
uniform float waterAlpha;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    vec3 texColor = vec3(texture2D(tex2D, mapCoord));

    // ambient
    vec3 ambient = light.ambient * material.ambient;
    
    // diffuse 
    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse * texColor);
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
        
    vec3 result = ambient + diffuse + specular;

	color = vec4(result, waterAlpha);
}
