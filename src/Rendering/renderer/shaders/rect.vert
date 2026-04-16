#version 330 core

layout(location = 0) in vec2 aQuadPos;   // [-1,1] x [-1,1]
layout(location = 1) in vec2 aCenter;    // per-instance
layout(location = 2) in vec2 aHalfExt;   // per-instance

uniform mat4 uTransform;
uniform float uPad;

out vec2 vLocalPos;

void main() {
    vLocalPos = aQuadPos * uPad;

    vec2 worldPos = aCenter + aQuadPos * (aHalfExt * uPad);
    gl_Position = uTransform * vec4(worldPos, 0.0, 1.0);
}