#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform vec3 Color;
uniform bool SelfLight;

void main()
{
    // ambient
    vec4 ambient = vec4(light.ambient, 1.0) * texture(material.diffuse, TexCoords);
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = vec4(light.diffuse * diff, 1.0) * texture(material.diffuse, TexCoords);  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec4 specular = vec4(light.specular * spec, 1.0) * texture(material.specular, TexCoords);  

    float theta = dot(lightDir, normalize(-light.direction));
    vec4 result;
    if (theta > light.cutOff){
        result = ambient + diffuse + specular;
    }
    else{
        result = ambient;
    }
    if (SelfLight){
        FragColor = vec4(Color, 1.0) * texture(material.diffuse, TexCoords);
    }
    else{
        FragColor = result;
    }
    
} 