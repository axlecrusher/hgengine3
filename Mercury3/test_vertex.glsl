#version 420
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 vertex;
layout(location = 3) in vec4 v_color;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];

out vec4 color;

void main() {
	gl_Position = matrices[PROJECTION] * matrices[VIEW] * matrices[MODEL] * vec4(vertex,1.0);
	color = v_color;
}


