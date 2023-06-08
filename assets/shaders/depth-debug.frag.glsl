#version 460 core

layout(binding = 0) uniform sampler2D U_SHADOW_MAP;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    float depth = texture(U_SHADOW_MAP, v_texCoord).r;
    fragColor = vec4(depth, depth, depth, 1.0);
}