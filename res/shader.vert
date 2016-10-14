#version 330 core

layout(location = 0) in vec3 vertexPos;

uniform mat4 modelToClip;

void main()
{
    gl_Position = modelToClip * vec4(vertexPos, 1.0);
}
