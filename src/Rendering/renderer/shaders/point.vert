#version 330 core

layout(location = 0) in vec2 aQuadPos;
layout(location = 1) in vec2 aCenter;
layout(location = 2) in float aSize;

uniform mat4 uTransform;

out vec2 vLocalPos;

void main() {
    vLocalPos = aQuadPos;

    vec2 worldPos = aCenter + aQuadPos * aSize;
    gl_Position = uTransform * vec4(worldPos, 0.0, 1.0);
}