#version 330 core

//Positions/Coordinates
layout(location = 0) in vec3 aPos;
// Outputs the color for the Fragment Shader
out vec3 color;

void main() {
    // Outputs the positions/coordinates of all vertices
    gl_Position = vec4(aPos.xyz, 1.0);
    // Assigns the colors from the Vertex Data to "color"
    color = vec3(
        0.5 + aPos.x,
        0.5 + aPos.y,
        0.5 + aPos.z
    );
}