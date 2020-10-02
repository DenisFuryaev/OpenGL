#version 330 core
out vec4 FragColor;

struct Matetial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform Matetial material;

in vec3 normal;
in vec3 frag_pos;

void main() 
{
    float ambient_strength = 0.1f;
    vec3 ambient = ambient_strength * light_color;

    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(light_dir, normal), 0.0f); 
    vec3 diffuse = diff * light_color; 

    float specular_strength = 0.5;
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0f), 32);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + diffuse  + specular) * object_color;

    FragColor = vec4(result , 1.0f);
}
