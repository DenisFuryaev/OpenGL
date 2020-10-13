#version 330 core
out vec4 FragColor;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

uniform sampler2D shadowMap;

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

float ComputeShadow() 
{
    vec3 pos = FragPosLightSpace.xyz * 0.5f + 0.5f;
    if (pos.z > 1.0f)
        pos.z = 1.0f;
    float depth = texture(shadowMap, pos.xy).r;
    float bias = 0.05f;
    return (depth + bias) < pos.z ? 0.0f : 1.0f;
}

void main() 
{
    
    vec3 normal = normalize(Normal);
    
    float ambient_strength = 0.55f;
    vec3 ambient = ambient_strength * light_color;

    vec3 light_dir = normalize(light_pos - FragPos);
    float diff = max(dot(light_dir, normal), 0); 
    vec3 diffuse = diff * light_color; 

    float specular_strength = 0.90;
    vec3 view_vector = view_pos - FragPos;
    vec3 view_dir = normalize(view_vector);   
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0f) , 32);
    vec3 specular = specular_strength * spec * light_color;


    float shadow = ComputeShadow();
    vec3 result = (ambient + shadow * (diffuse + specular)) * object_color;
    
    FragColor = vec4(result , 1.0f);
}





/* DEPTH VALUE REPRESENTATION
float near = 0.1; 
float far  = 100.0; 
float depth = gl_FragCoord.z;
float ndc = depth * 2.0 - 1.0; 
float linearDepth = (2.0 * near * far) / (far + near - ndc * (far - near));
vec3 result = vec3(linearDepth) / far * 2;
*/