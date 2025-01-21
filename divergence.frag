#version 330 core

uniform sampler2D w; //vector field
uniform float halfrdx;


in vec2 texCoords;
out vec4 fragColor;

void main() {
    vec4 wL = texture(w, texCoords - vec2(1.0, 0.0));
    vec4 wR = texture(w, texCoords + vec2(1.0, 0.0));
    vec4 wB = texture(w, texCoords - vec2(0.0, 1.0));
    vec4 wT = texture(w, texCoords + vec2(0.0, 1.0));

    fragColor = vec4(halfrdx * ((wR.x - wL.x) + (wT.y - wB.y)), 0.0, 0.0, 0.0);
}