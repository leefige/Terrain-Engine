/*
 * GLSL Fragment Shader for terrain.
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

uniform sampler2D texColor;
uniform sampler2D texDetail;

uniform float detailScale;
uniform float upY;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool useLight;

void main()
{
	// this part is under the visiable plane
	float yPos = FragPos.y;
	if (upY * yPos < 0) {
		discard;
	}

	vec4 myColor = texture2D(texColor, mapCoord);
	vec4 myDetail = texture2D(texDetail, detailScale * mapCoord);
	// GL_ADD_SIGNED: a + b - 0.5
	vec4 result = myColor + myDetail - 0.5f;

    if (useLight) {
	    // ambient
        vec3 ambient = light.ambient * material.ambient;
    
        // diffuse 
        vec3 norm = normalize(Normal);

        vec3 lightDir = normalize(light.position);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * (spec * material.specular);  
        
        result = vec4(sqrt(ambient + diffuse + specular) * result.rgb, result.a);
    }
    color = result;
}
