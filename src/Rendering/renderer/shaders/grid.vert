#version 330 core

layout (location = 0) in vec2 aPos;

out vec2 vNdcPos;

void main() {
    vNdcPos = aPos;
    gl_Position = vec4(aPos, 0.0, 1.0);
}