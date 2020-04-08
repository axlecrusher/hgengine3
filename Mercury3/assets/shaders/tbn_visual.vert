#version 420

layout(location = 0) in vec3 vertex;
layout(location = 3) in vec4 v_color;

layout(location = 5) in mat4 ModelMatrix;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];

out vec4 color;
out vec3 fragNormal;
out vec3 fragPos;
out vec3 eyePos;

void main() {
	int bufferOffset = gl_InstanceID*4;

	eyePos = matrices[VIEW][3].xyz*-1.0; //pull eye position from transform matrix

	vec4 worldSpacePos =  ModelMatrix * vec4(vertex,1);
	gl_Position = matrices[PROJECTION] * matrices[VIEW] * worldSpacePos;
	color = v_color;
}


