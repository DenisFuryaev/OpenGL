#version 330 core
out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

uniform float far_plane;
uniform sampler2D diffuse_texture1;
uniform sampler2D normal_texture1;
uniform sampler2D specular_texture1; // height, not specular
uniform samplerCube depthMap;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;

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

vec2 ParallaxMap(vec2 texCoords, vec3 viewDir)
{
    float height = 1 - texture(specular_texture1, texCoords).r;
    vec2 p = viewDir.xy * (height * 0.04f);
    return texCoords - p;
}

void main() 
{
    
    
    float ambient_strength = 0.5f;

    vec3 viewDir = normalize(TBN * view_pos - TBN * FragPos);
    vec2 newTexCoords = ParallaxMap(TexCoords, viewDir);
    if (newTexCoords.x > 1.0 || newTexCoords.y > 1.0 || newTexCoords.x < 0.0 || newTexCoords.y < 0.0)
        discard;
    vec3 ambient_color = texture(diffuse_texture1, newTexCoords).rgb;
    //vec3 ambient_color = texture(diffuse_texture1, TexCoords).rgb;

    vec3 normal = texture(normal_texture1, newTexCoords).rgb;
    // from color to coordinates
    normal = normal * 2.0 - 1.0;
    // from tangent to world space
    normal = normalize(TBN * normal);

    vec3 ambient = ambient_strength * ambient_color  * light_color;

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

   
