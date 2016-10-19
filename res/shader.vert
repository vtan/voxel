#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in float brightness;

varying float vertBrightness;

uniform mat4 modelToClip;

void main()
{
    vertBrightness = brightness;
    gl_Position = modelToClip * vec4(position, 1.0);
}
