#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

layout(binding = 0)uniform sampler2D gPosition;
layout(binding = 1)uniform sampler2D gNormal;
layout(binding = 2)uniform sampler2D gAlbedoSpec;

void main() {
    FragColor = vec4(texture(gPosition, TexCoords).rgb, 1.0);
}