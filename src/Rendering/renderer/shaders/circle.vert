#version 330 core

layout(location = 0) in vec2 aWorldPos;
layout(location = 1) in vec2 aLocalPos;

uniform mat4 uTransform;

out vec2 vLocalPos;

void main() {
    vLocalPos = aLocalPos;
    gl_Position = uTransform * vec4(aWorldPos, 0.0, 1.0);
}