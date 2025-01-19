#version 330 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D velocityTexture;
uniform vec2 mousePos;      // Normalized mouse position [0, 1]
uniform vec2 forceDir;      // Direction of the force
uniform float forceRadius;  // Radius of the force application
uniform float forceStrength;

void main() {
    vec2 uv = texCoords;

    // Calculate distance from the mouse position
    float distance = length(uv - mousePos);

    // Apply force within the radius
    if (distance < forceRadius) {
//        float influence = smoothstep(forceRadius, 0.0, distance); // Decay force with distance
        float influence = distance / forceRadius;
        vec2 currentVelocity = texture(velocityTexture, uv).xy;
        vec2 newVelocity = currentVelocity + influence * forceDir * forceStrength;
//        vec2 newVelocity = currentVelocity + forceDir * forceStrength;

        FragColor = vec4(newVelocity, 0.0, 1.0);
    } else {
        FragColor = texture(velocityTexture, uv);
    }
}
