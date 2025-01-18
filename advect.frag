#version 330 core

uniform sampler2D dyeTexture;
uniform sampler2D velocityTexture;
uniform float timestep;
uniform float rdx;
uniform bool isAdvectDye;


in vec2 texCoords;

out vec4 fragColor;

void main() {
    vec2 newX = texCoords - timestep * rdx * texture(velocityTexture, texCoords).xy;
//    newX = clamp(newX, vec2(0.0), vec2(1.0));

    vec4 advectedValue;
    if (isAdvectDye) {
        advectedValue = texture(dyeTexture, newX);
    } else {
        advectedValue = texture(velocityTexture, newX);
    }

    fragColor = advectedValue;

}