#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D velocityTexture;
uniform vec2 mousePos;      // Normalized mouse position [0, 1]
uniform vec2 forceDir;      // Direction of the force
uniform float forceRadius;  // Radius of the force application
uniform float forceStrength;
uniform float gridSize;

void main() {
    ivec2 gridCellIndex = ivec2(floor(texCoords * gridSize));

    if (gridCellIndex.x == 0 || gridCellIndex.x == gridSize - 1 ||
    gridCellIndex.x == 0 || gridCellIndex.x == gridSize - 1) { //Boundary: dont apply force
        fragColor = texture(velocityTexture, texCoords);
    } else {
        // Calculate distance from the mouse position
        float distance = length(texCoords - mousePos);

        // Apply force within the radius
        if (distance < forceRadius) {
            float influence = exp(-distance * distance / (2.0 * forceRadius * forceRadius));
            vec2 currentVelocity = texture(velocityTexture, texCoords).xy;
//            vec2 newVelocity = currentVelocity + influence * forceDir * forceStrength;
            vec2 newVelocity = currentVelocity + influence * normalize(texCoords) * forceStrength; // radial direction

            fragColor = vec4(newVelocity, 0.0, 1.0);
        } else {
            fragColor = texture(velocityTexture, texCoords);
        }
    }

}
