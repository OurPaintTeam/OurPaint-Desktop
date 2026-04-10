#version 330 core

in vec2 vLocalPos;

uniform vec3 uColor;

out vec4 FragColor;

void main() {
    float d = abs(vLocalPos.y);           // 0 in center, 1 at side edge
    float aa = max(fwidth(d), 1e-4);
    float alpha = 1.0 - smoothstep(1.0 - aa, 1.0, d);

    if (alpha <= 0.0) {
        discard;
    }

    FragColor = vec4(uColor, alpha);
}