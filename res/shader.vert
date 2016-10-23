#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in float brightness;

out float vertDiffuseLight;
out float vertAmbientLight;

uniform mat4 modelToClip;

const vec3 dirToLight = normalize(vec3(0.2, 1.0, 0.0));

void main()
{
    vertDiffuseLight = max(0.0, dot(normal, dirToLight));
    vertAmbientLight = brightness;

    gl_Position = modelToClip * vec4(position, 1.0);
}
