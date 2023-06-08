#version 460

layout(location = 0) in vec3 POSITION;

uniform mat4 U_MVP;

void main() {
    gl_Position = U_MVP * vec4(POSITION, 1.0);
}