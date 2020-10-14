#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool reverse_normals;

out vec3 Normal;
out vec3 FragPos;


void main()
{

    if (reverse_normals) 
        Normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
    else
        Normal = transpose(inverse(mat3(model))) * aNormal;

    FragPos = vec3(model * vec4(aPos, 1.0f));
  
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
