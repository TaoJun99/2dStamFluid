#version 330 core

uniform sampler2D w; //vector field
uniform float halfrdx;
uniform int gridSize;

in vec2 texCoords;
out vec4 fragColor;

void main() {

    ivec2 texCoordInt = ivec2(texCoords * gridSize);  // Convert normalized to integer coordinates

    // Fetch neighboring texels
    vec4 wL = texelFetch(w, texCoordInt - ivec2(1, 0), 0);  // Left
    vec4 wR = texelFetch(w, texCoordInt + ivec2(1, 0), 0);  // Right
    vec4 wB = texelFetch(w, texCoordInt - ivec2(0, 1), 0);  // Bottom
    vec4 wT = texelFetch(w, texCoordInt + ivec2(0, 1), 0);  // Top

    fragColor = vec4(halfrdx * ((wR.x - wL.x) + (wT.y - wB.y)), 0.0, 0.0, 0.0);
}