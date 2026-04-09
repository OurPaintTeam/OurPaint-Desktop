#version 330 core

in vec2 vLocalPos;

uniform vec3 uColor;

out vec4 FragColor;

void main() {
    float r = length(vLocalPos);
    float sd = abs(r - 1.0);           // distance to ideal circle border in local units
    float px = fwidth(r);              // local-units per pixel approximately

    float alpha = 1.0 - smoothstep(0.0, px, sd);

    if (alpha <= 0.0) {
        discard;
    }

    FragColor = vec4(uColor, alpha);
}