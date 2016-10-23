#version 330 core

in float vertDiffuseLight;
in float vertAmbientLight;

out vec4 fragColor;

void main()
{
    float brightness =
        0.5 * vertAmbientLight
        + 0.5 * vertDiffuseLight;
    fragColor = vec4(brightness * vec3(1.0), 1.0);
}
