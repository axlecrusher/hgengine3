#version 420

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 v_color;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];
uniform samplerBuffer bufferObject1;

out vec4 color;
out vec3 fragNormal;
out vec3 fragPos;
out vec3 eyePos;

void main() {
	int bufferOffset = gl_InstanceID*4;

	mat4 modelMatrix;

	modelMatrix[0] = texelFetch(bufferObject1,bufferOffset);
	modelMatrix[1] = texelFetch(bufferObject1,bufferOffset+1);
	modelMatrix[2] = texelFetch(bufferObject1,bufferOffset+2);
	modelMatrix[3] = texelFetch(bufferObject1,bufferOffset+3);
//modelMatrix = mat4(1.0);

	eyePos = matrices[VIEW][3].xyz*-1.0; //pull eye position from transform matrix

	vec4 worldSpacePos =  modelMatrix * vec4(vertex,1);
	gl_Position = matrices[PROJECTION] * matrices[VIEW] * worldSpacePos;
	color = v_color;
}


