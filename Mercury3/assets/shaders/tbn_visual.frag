#version 420

out vec4 fragColor;

in vec4 color;
in vec3 fragNormal;
in vec3 eyePos;
in vec3 fragPos;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];

void main() {
  fragColor = color;
}