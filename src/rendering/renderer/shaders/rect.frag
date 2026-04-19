#version 330 core

in vec2 vLocalPos;

uniform vec4 uFillColor;
uniform vec4 uBorderColor;
uniform float uBorderHalfWidth;
uniform float uEdgeSoftness;

out vec4 FragColor;

void main() {
//    float outer = max(abs(vLocalPos.x), abs(vLocalPos.y));
//
//    // Fill: fully inside rectangle, fade near outer edge
//    float fillAlpha = 1.0 - smoothstep(1.0, 1.0 + uEdgeSoftness, outer);
//
//    // Border: ring around outer = 1
//    float borderDist = abs(outer - 1.0);
//    float borderAlpha = 1.0 - smoothstep(
//        uBorderHalfWidth,
//        uBorderHalfWidth + uEdgeSoftness,
//        borderDist
//    );
//
//    vec4 fill = vec4(uFillColor.rgb, uFillColor.a * fillAlpha);
//    vec4 border = vec4(uBorderColor.rgb, uBorderColor.a * borderAlpha);
//
//    // Border over fill
//    vec4 color = fill;
//    color.rgb = mix(color.rgb, border.rgb, border.a);
//    color.a = max(color.a, border.a);
//
//    if (color.a <= 0.0) {
//        discard;
//    }

    vec4 color = uFillColor;

    FragColor = color;
}