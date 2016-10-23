#version 330 core

out vec4 fragColor;

varying float vertDiffuseLight;
varying float vertAmbientLight;

void main()
{
    float brightness =
        0.5 * vertAmbientLight
        + 0.5 * vertDiffuseLight;
    fragColor = vec4(brightness * vec3(1.0), 1.0);
}
