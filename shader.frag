#version 330 core
out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D inputTexture;

void main() {
    vec4 color = texture(inputTexture, texCoords);
    FragColor = color;
}