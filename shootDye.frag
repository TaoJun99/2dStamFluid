#version 330 core

uniform sampler2D dyeTexture;
uniform sampler2D velocityTexture;
uniform sampler2D outputTexture;
uniform vec3 dyeColor;
uniform bool isAddDye;
uniform float time;

in vec2 texCoords;
out vec4 fragColor;


float randAngle(vec2 seed, float time) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453 + time);
}

void main() {

    if (texCoords.x >= 0.5 && texCoords.x <= 0.502 && texCoords.y >= 0.5 && texCoords.y <= 0.502) {
        if (isAddDye) { //dye
            vec4 currentDye = texture(dyeTexture, texCoords);
            fragColor = vec4(currentDye.rgb + dyeColor, 1.0);
        } else { //velocity
            vec4 currentVelocity = texture(velocityTexture, texCoords);

            // Generate a random angle for the velocity direction
            float angle = randAngle(texCoords, time) * 6.2831853; // Random angle [0, 2*PI]

            // Calculate the random velocity direction (unit vector)
            vec2 randomDirection = vec2(cos(angle), sin(angle));
            float strength = 10.0;
            fragColor = vec4(currentVelocity.xy + strength * (vec2(1.0, 5.0) + randomDirection), 0.0, 0.0);
        }
    } else { // Dont change anything
        fragColor = texture(outputTexture, texCoords);
    }
}