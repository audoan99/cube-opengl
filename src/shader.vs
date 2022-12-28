#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 inColor;

uniform mat4 gRotation;

out vec4 Color;

void main()
{
    gl_Position = gRotation * vec4(Position, 1.0);
    Color = vec4(inColor, 1.0);
}