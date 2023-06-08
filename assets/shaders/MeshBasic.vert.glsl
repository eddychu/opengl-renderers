#version 460

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
layout(location = 3) in vec3 TANGENT;
layout(location = 4) in vec3 BITANGENT;


uniform mat4 PROJ_MAT4;
uniform mat4 VIEW_MAT4;
uniform mat4 MODEL_MAT4;
uniform mat3 NORMAL_MAT3;



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
    gl_Position = PROJ_MAT4 * VIEW_MAT4 * MODEL_MAT4 * vec4(POSITION, 1.0);
    vs_out.position = vec3(MODEL_MAT4 * vec4(POSITION, 1.0));
    vs_out.normal = NORMAL_MAT3 * NORMAL;
    vs_out.uv = TEXCOORD;
    vs_out.tangent = NORMAL_MAT3 * TANGENT;
    vs_out.bitangent = NORMAL_MAT3 * BITANGENT;
}