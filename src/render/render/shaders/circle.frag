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

float normalizeAngle(float a) {
    a = mod(a, TAU);
    return (a < 0.0) ? (a + TAU) : a;
}

bool angleInArc(float angle, float startA, float endA) {
    angle  = normalizeAngle(angle);
    startA = normalizeAngle(startA);
    endA   = normalizeAngle(endA);

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

    float ringAlpha = 1.0 - smoothstep(
        vHalfWidthLocal,
        vHalfWidthLocal + vEdgeSoftnessLocal,
        sd
    );

    float angle = atan(vLocalPos.y, vLocalPos.x);
    angle = normalizeAngle(angle);

    if (!angleInArc(angle, vStartAngle, vEndAngle))
        discard;

    FragColor = vec4(uColor, ringAlpha);
}

//void main() {
//    float r = length(vLocalPos);
//    float sd = abs(r - 1.0);
//
//    float alpha = 1.0 - smoothstep(vHalfWidthLocal, vHalfWidthLocal + vEdgeSoftnessLocal, sd);
//
//    FragColor = vec4(uColor, alpha);
//}