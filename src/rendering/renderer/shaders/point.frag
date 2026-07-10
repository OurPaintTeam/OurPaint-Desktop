#version 330 core

in vec2 vLocalPos;

uniform vec3 uColor;
uniform float uEdgeSoftness;

out vec4 FragColor;

void main() {
    float d = length(vLocalPos);
    float alpha;
    if (uEdgeSoftness <= 0.0) {
        alpha = d <= 1.0 ? 1.0 : 0.0;
    } else {
        alpha = 1.0 - smoothstep(1.0, 1.0 + uEdgeSoftness, d);
    }
    FragColor = vec4(uColor, alpha);
}

