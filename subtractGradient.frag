#version 330 core

uniform sampler2D p; // pressure field
uniform sampler2D w; // velocity
uniform float halfrdx;


in vec2 texCoords;
out vec4 fragColor;

void main() {
    float pL = texture(p, texCoords - vec2(1.0, 0.0)).x;
    float pR = texture(p, texCoords + vec2(1.0, 0.0)).x;
    float pB = texture(p, texCoords - vec2(0.0, 1.0)).x;
    float pT = texture(p, texCoords + vec2(0.0, 1.0)).x;

    fragColor = texture(w, texCoords);
    fragColor.xy = fragColor.xy - halfrdx * vec2(pR - pL, pT - pB);
}