#version 330 core
out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

uniform float far_plane;
uniform sampler2D diffuse_texture1;
uniform samplerCube depthMap;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

float ComputeShadow() 
{
    vec3 lightToFrag = FragPos - light_pos;

    float depth = texture(depthMap, lightToFrag).r;

    // if object is out of the frustum return 1, so there is no dark region out of the fov of shadow perspective projection 
    if (depth == 1)
        return 1.0f;

    depth *= far_plane;

    float bias = 0.1f;
    float delta = length(lightToFrag) - (depth + bias);

    if (delta > 0) 
        return 0.0f;
    else
        return 1.0f;
}

void main() 
{
    
    vec3 normal = normalize(Normal);
    
    float ambient_strength = 0.55f;

    vec3 ambient_color = texture(diffuse_texture1, TexCoords).rgb;

    vec3 ambient = 0.4 * ambient_color  * light_color;

    vec3 light_dir = normalize(light_pos - FragPos);
    float diff = max(dot(light_dir, normal), 0); 
    vec3 diffuse = diff * light_color; 

    float specular_strength = 0.40;
    vec3 view_vector = view_pos - FragPos;
    vec3 view_dir = normalize(view_vector);   
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0f) , 32);
    vec3 specular = specular_strength * spec * light_color;


    float shadow = ComputeShadow();
    vec3 result = (ambient + shadow * (diffuse + specular)) * ambient_color;

    FragColor = vec4(result , 1.0f);
}