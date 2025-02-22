#version 330 core

uniform sampler2D p; // pressure field
uniform sampler2D w; // velocity
uniform float halfrdx;
uniform int gridSize;


in vec2 texCoords;
out vec4 fragColor;

void main() {
    ivec2 texCoordInt = ivec2(texCoords * gridSize);

    float pL = texelFetch(p, texCoordInt - ivec2(1, 0), 0).x;  // Left
    float pR = texelFetch(p, texCoordInt + ivec2(1, 0), 0).x;  // Right
    float pB = texelFetch(p, texCoordInt - ivec2(0, 1), 0).x;  // Bottom
    float pT = texelFetch(p, texCoordInt + ivec2(0, 1), 0).x;  // Top

    fragColor = texelFetch(w, texCoordInt, 0);
    fragColor.xy = fragColor.xy - halfrdx * vec2(pR - pL, pT - pB);
}