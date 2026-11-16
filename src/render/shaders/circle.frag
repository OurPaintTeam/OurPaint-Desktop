#version 330 core

in vec2 vLocalPos;
in float vHalfWidthLocal;
in float vEdgeSoftnessLocal;
in float vStartAngle;
in float vEndAngle;

uniform vec3 uColor;

out vec4 FragColor;

const float PI  = 3.14159265358979323846;
const float TAU = 6.28318530717958647692;

bool angleInArc(float angle, float startA, float endA) {
    angle  = mod(angle, TAU);
    startA = mod(startA, TAU);
    endA   = mod(endA, TAU);

    float span = mod(endA - startA + TAU, TAU);

    if (span < 1e-5)
        return true;

    if (startA <= endA) {
        return angle >= startA && angle <= endA;
    } else {
        return angle >= startA || angle <= endA;
    }
}


void main() {
    float r = length(vLocalPos);
    float sd = abs(r - 1.0);

    float angle = atan(vLocalPos.y, vLocalPos.x);
    angle = mod(angle, TAU);

    if (!angleInArc(angle, vStartAngle, vEndAngle))
        discard;

    float ringAlpha = 1.0 - smoothstep(
        vHalfWidthLocal,
        vHalfWidthLocal + vEdgeSoftnessLocal,
        sd
    );

    FragColor = vec4(uColor, ringAlpha);
}
