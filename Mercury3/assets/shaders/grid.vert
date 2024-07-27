#version 420
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 vertex;
layout(location = 3) in vec4 v_color;
layout(location = 5) in mat4 ModelMatrix;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];

out vec4 color;
out vec4 fragCoord;
out vec3 camPos;

//http://madebyevan.com/shaders/grid/
void main() {
	camPos = ModelMatrix[3].xyz; //camera or eye position


	const mat3 modelRot = mat3(ModelMatrix);
	vec4 worldSpace =  ModelMatrix * vec4(vertex,1);

	//Is this broken in stereo view?
	gl_Position = matrices[PROJECTION] * matrices[VIEW] * worldSpace;

	color = v_color;
	fragCoord=worldSpace;
}


