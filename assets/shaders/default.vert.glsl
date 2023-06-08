#version 460

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
layout(location = 3) in vec3 TANGENT;
layout(location = 4) in vec3 BITANGENT;


uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;




out VS_OUT
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec3 tangent;
    vec3 bitangent;
} vs_out;

void main()
{
    gl_Position = MVP * vec4(POSITION, 1.0);
    vs_out.position = vec3(ModelMatrix * vec4(POSITION, 1.0));
    vs_out.normal = NormalMatrix * NORMAL;
    vs_out.uv = TEXCOORD;
    vs_out.tangent = NormalMatrix * TANGENT;
    vs_out.bitangent = NormalMatrix * BITANGENT;
}