#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 ClipCoord;

void main()
{
    ClipCoord = projection * view * model * vec4(aPos, 1.0);
    gl_Position = ClipCoord;
}

