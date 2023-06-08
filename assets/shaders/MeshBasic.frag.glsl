#version 460

out vec4 FragColor;

uniform vec3 U_COLOR;

in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
    vec3 bitangent;
} vs_out;

void main()
{
    FragColor = vec4(U_COLOR, 1.0);
}