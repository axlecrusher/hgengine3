#version 420

//uniform vec4 inputColour;
out vec4 fragColour;
varying vec3 color;

void main() {
  fragColour = vec4(color,1.0);
}