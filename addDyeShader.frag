#version 330 core

uniform sampler2D dyeTexture;
uniform vec2 mousePos;
uniform float dyeRadius;
uniform vec3 dyeColor;
uniform bool addDye;

out vec4 fragColor;

void main() {
    // Normalized texture coordinates
    vec2 uv = gl_FragCoord.xy / textureSize(dyeTexture, 0);
    // Current color of fragment
    vec4 currentColor = texture(dyeTexture, uv);

    // Distance of fragment from position where dye is dropped
    float dist = distance(uv, mousePos);

    // Fragment within dyeRadius
    if (addDye && dist < dyeRadius) {
        float colorIntensity = 1.0 - dist / dyeRadius;
        vec3 addedDye = dyeColor * colorIntensity;
        currentColor.rgb += addedDye;
    }

    fragColor = vec4(currentColor.rgb, 0.1);
}