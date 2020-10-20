#version 330 core
out vec4 FragColor;

uniform samplerCube skybox;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 view_pos;

void main()
{
    vec3 normal = normalize(Normal);
    vec3 view = normalize(view_pos - FragPos);
    vec3 reflection = 2 * dot(normal, view) * normal - view;

    float refravtive_index = 0.62f; // for glass
    float cos_angle = dot(normal, view);
    float sin_angle = sqrt(1 - cos_angle * cos_angle);
    vec3 refraction =  refravtive_index * -view + (refravtive_index * cos_angle - sqrt(1 - (refravtive_index * refravtive_index * sin_angle * sin_angle))) * normal;
    //vec3 refraction = refract(view, normal, refravtive_index);
    FragColor = texture(skybox, refraction) ;
}
