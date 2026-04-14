#version 330 core

in vec2 vLocalPos;
in float vHalfWidthLocal;
in float vEdgeSoftnessLocal;

uniform vec3 uColor;

out vec4 FragColor;

void main() {
    float r = length(vLocalPos);
    float sd = abs(r - 1.0);

    float alpha = 1.0 - smoothstep(vHalfWidthLocal, vHalfWidthLocal + vEdgeSoftnessLocal, sd);

    FragColor = vec4(uColor, alpha);
}