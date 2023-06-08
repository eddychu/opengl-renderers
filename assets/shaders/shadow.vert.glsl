#version 460

layout(location = 0) in vec3 POSITION;

uniform mat4 U_WVP;

void main() {
    gl_Position = U_WVP * vec4(POSITION, 1.0);
}