#version 330 core

uniform vec4 uFillColor;

out vec4 FragColor;

void main() {
    vec4 color = uFillColor;

    FragColor = color;
}