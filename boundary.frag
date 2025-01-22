#version 330 core

uniform float scale;
uniform sampler2D texture;

in vec2 texCoords;
out vec4 fragColor;

void main() {
    ivec2 texCoordInt = ivec2(texCoords * 128);

    ivec2 offset = ivec2(0, 0);

    if (texCoordInt.x == 0) {
        if (texCoordInt.y != 0 && texCoordInt.y != 127) {
            offset = ivec2(1, 0);
        }
    } else if (texCoordInt.x == 127) {
        if (texCoordInt.y != 0 && texCoordInt.y != 127) {
            offset = ivec2(-1, 0);
        }
    } else if (texCoordInt.y == 0) {
        if (texCoordInt.x != 0 && texCoordInt.x != 127) {
            offset = ivec2(0, 1);
        }
    } else if (texCoordInt.y == 127) {
        if (texCoordInt.x != 0 && texCoordInt.x != 127) {
            offset = ivec2(0, -1);
        }
    }

    if (offset == ivec2(0,0)) {
        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        fragColor = scale * texelFetch(texture, texCoordInt + offset, 0);
    }
}
