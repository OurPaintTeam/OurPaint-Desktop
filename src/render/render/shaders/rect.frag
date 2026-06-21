#version 330 core

in vec2 vLocalPos; // [-1, 1]

uniform bool  uHasFill;
uniform bool  uHasStroke;

uniform vec4  uFillColor;
uniform vec4  uStrokeColor;
uniform float uStrokeWidthPx;

out vec4 FragColor;

vec4 compositeOver(vec4 foreground, vec4 background) {
    float outAlpha = foreground.a + background.a * (1.0 - foreground.a);

    if (outAlpha <= 0.0) {
        return vec4(0.0);
    }

    vec3 outRgb = (foreground.rgb * foreground.a + background.rgb * background.a * (1.0 - foreground.a)) / outAlpha;

    return vec4(outRgb, outAlpha);
}

void main() {
    vec4 color = uHasFill ? uFillColor : vec4(0.0);

    if (uHasStroke) {
        float distanceX = (1.0 - abs(vLocalPos.x)) / max(fwidth(vLocalPos.x), 1e-6);

        float distanceY = (1.0 - abs(vLocalPos.y)) / max(fwidth(vLocalPos.y), 1e-6);

        float distanceToBorderPx = min(distanceX, distanceY);

        float strokeCoverage = 1.0 - smoothstep(uStrokeWidthPx - 0.5, uStrokeWidthPx + 0.5, distanceToBorderPx);

        vec4 stroke = uStrokeColor;
        stroke.a *= strokeCoverage;

        color = compositeOver(stroke, color);
    }

    if (color.a <= 0.0) {
        discard;
    }

    FragColor = color;
}