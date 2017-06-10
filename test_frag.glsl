#version 420

//uniform vec4 inputColour;
out vec4 fragColour;
varying vec4 color;

void main() {
  fragColour = color;
}