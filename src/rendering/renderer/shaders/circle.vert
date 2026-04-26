#version 330 core

layout(location = 0) in vec2 aQuadPos;
layout(location = 1) in vec2 aCenter;
layout(location = 2) in float aRadius;

layout(location = 3) in float aStartAngle; // new
layout(location = 4) in float aEndAngle; // new

uniform mat4 uTransform;
uniform float uZoom;
uniform float uCurveHalfWidthPx;
uniform float uEdgeSoftnessPx;

out vec2 vLocalPos;
out float vHalfWidthLocal;
out float vEdgeSoftnessLocal;
out float vStartAngle;
out float vEndAngle;

void main() {
    float radiusPx = max(aRadius * uZoom, 1e-6);

    float halfWidthLocal = uCurveHalfWidthPx / radiusPx;
    float edgeSoftnessLocal = uEdgeSoftnessPx / radiusPx;
    float pad = 1.0 + halfWidthLocal + edgeSoftnessLocal;

    vLocalPos = aQuadPos * pad;
    vHalfWidthLocal = halfWidthLocal;
    vEdgeSoftnessLocal = edgeSoftnessLocal;
    vStartAngle = aStartAngle;
    vEndAngle = aEndAngle;

    vec2 worldPos = aCenter + aQuadPos * (aRadius * pad);
    gl_Position = uTransform * vec4(worldPos, 0.0, 1.0);
}