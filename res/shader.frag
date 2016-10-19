#version 330

out vec4 fragColor;

varying float vertBrightness;

void main()
{
    fragColor = vec4(vertBrightness * vec3(1.0), 1.0);
}
