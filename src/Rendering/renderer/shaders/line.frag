#version 330 core

in vec2 vLocalPos;

uniform vec3 uColor;
uniform float uAlpha;
uniform float uEdgeSoftness;

out vec4 FragColor;

void main() {
    float d = abs(vLocalPos.y);
    float alpha = 1.0 - smoothstep(1.0, 1.0 + uEdgeSoftness, d);
    alpha *= uAlpha;

    FragColor = vec4(uColor, alpha);
}