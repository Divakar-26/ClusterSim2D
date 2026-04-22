#version 330 core

layout(location = 0) in vec2 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 localPos;

void main()
{
    localPos = aPos; // 0 → 1 quad space
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
}  