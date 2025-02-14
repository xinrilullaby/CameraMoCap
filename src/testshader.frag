#version 330 core

// Inputs the color from the Vertex Shader
in vec3 color;
// Outputs colors in RGBA
out vec4 FragColor;

void main() {
    FragColor = vec4(color, 1.0f);
}