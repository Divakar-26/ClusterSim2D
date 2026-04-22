#version 330 core

out vec4 FragColor;

uniform vec4 color;
uniform int isCircle;

in vec2 localPos;

void main()
{
    if(isCircle == 1)
    {
        float dist = length(localPos - vec2(0.5));

        if(dist > 0.5) 
            discard;
    } 

    FragColor = color;
}