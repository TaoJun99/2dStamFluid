#version 330 core

uniform sampler2D advectedTexture;
uniform sampler2D velocityTexture;
uniform float timestep;
uniform float rdx;

in vec2 texCoords;

out vec4 fragColor;

void main() {
    vec2 newX = texCoords - timestep * rdx * texture(velocityTexture, texCoords).xy;

    vec4 advectedValue = texture(advectedTexture, newX);

    fragColor = advectedValue;
}