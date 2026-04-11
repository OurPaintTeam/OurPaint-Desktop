#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat4 uTransform;

out vec2 vQuadPos;

void main() {
    vQuadPos = aPos;
    gl_Position = vec4(aPos, 0.0, 1.0);
}