#version 420

uniform samplerBuffer bufferObject1;

//uniform vec4 inputColour;
out vec4 fragColour;
varying vec4 color;

void main() {
  fragColour = color;
}