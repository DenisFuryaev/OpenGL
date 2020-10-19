#version 330 core
out vec4 FragColor;

uniform samplerCube skybox;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 view_pos;

void main()
{
    vec3 normal = normalize(Normal);
    vec3 FragToView = normalize(view_pos - FragPos);
    vec3 reflection_vector = 2 * dot(normal, FragToView) * normal - FragToView;

    FragColor = texture(skybox, reflection_vector);
}
