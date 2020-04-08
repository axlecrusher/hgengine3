#version 420
#extension GL_ARB_explicit_uniform_location : enable

//layout of the vertex buffer
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv_coord;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];
//uniform samplerBuffer bufferObject1;
in mat4 ModelMatrix;

out vec2 frag_uv;

void main() {
	int bufferOffset = gl_InstanceID*4;

	vec4 worldSpacePos =  ModelMatrix * vec4(vertex,1);
	gl_Position = matrices[PROJECTION] * matrices[VIEW] * worldSpacePos;
//	gl_Position = vec4(vertex,1);

	frag_uv = uv_coord.xy;
}


