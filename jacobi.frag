#version 330 core

uniform float alpha;
uniform float rBeta;
uniform float dx;
uniform sampler2D x;
uniform sampler2D b;

in vec2 texCoords;

out vec4 fragColor;

void main() {
    // 1 Jacobi update iteration
//    vec4 xL = texture(x, texCoords - vec2(1.0, 0.0));
//    vec4 xR = texture(x, texCoords + vec2(1.0, 0.0));
//    vec4 xB = texture(x, texCoords - vec2(0.0, 1.0));
//    vec4 xT = texture(x, texCoords + vec2(0.0, 1.0));

    ivec2 texCoordInt = ivec2(texCoords * 128);  // Convert normalized to integer coordinates

    // Fetch neighboring texels
    vec4 xL = texelFetch(x, texCoordInt - ivec2(1, 0), 0);  // Left
    vec4 xR = texelFetch(x, texCoordInt + ivec2(1, 0), 0);  // Right
    vec4 xB = texelFetch(x, texCoordInt - ivec2(0, 1), 0);  // Bottom
    vec4 xT = texelFetch(x, texCoordInt + ivec2(0, 1), 0);  // Top

    vec4 bC = texture(b, texCoords);

    fragColor = (xL + xR + xB + xT + alpha * bC) * rBeta;
}