#version 330 core

layout(location = 0) in vec2 aQuadPos;
layout(location = 1) in vec2 aCenter;
layout(location = 2) in float aRadius;

uniform mat4 uTransform;

out vec2 vLocalPos;

void main() {
    float pad = 1.05;
    vLocalPos = aQuadPos * pad;

    vec2 worldPos = aCenter + aQuadPos * (aRadius * pad);
    gl_Position = uTransform * vec4(worldPos, 0.0, 1.0);
}