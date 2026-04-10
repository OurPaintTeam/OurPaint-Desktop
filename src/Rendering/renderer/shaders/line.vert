#version 330 core

layout(location = 0) in vec2 aQuadPos;    // x in [0,1], y in [-1,1]
layout(location = 1) in vec2 aP0;
layout(location = 2) in vec2 aP1;
layout(location = 3) in float aHalfWidth;

uniform mat4 uTransform;

out vec2 vLocalPos;

void main() {
    vec2 delta = aP1 - aP0;
    float len = length(delta);

    vec2 dir = (len > 1e-6) ? (delta / len) : vec2(1.0, 0.0);
    vec2 normal = vec2(-dir.y, dir.x);

    vec2 worldPos = aP0
                  + dir * (aQuadPos.x * len)
                  + normal * (aQuadPos.y * aHalfWidth);

    gl_Position = uTransform * vec4(worldPos, 0.0, 1.0);

    // normalized local coordinates for fragment shader
    vLocalPos = vec2(aQuadPos.x, aQuadPos.y);
}