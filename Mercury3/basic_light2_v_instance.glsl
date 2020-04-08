#version 420

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv_coord;
layout(location = 3) in vec4 v_color;

layout(location = 5) in mat4 ModelMatrix;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];
//uniform samplerBuffer bufferObject1;

out vec4 color;
out vec3 fragNormal;
out vec3 fragPos;
out vec3 eyePos;

void main() {
	int bufferOffset = gl_InstanceID*4;

	vec3 worldSpaceNormal = normalize(mat3(ModelMatrix) * normal.xyz);
	fragNormal = worldSpaceNormal;

	eyePos = matrices[VIEW][3].xyz*-1.0; //pull eye position from transform matrix

	vec4 worldSpacePos =  ModelMatrix * vec4(vertex,1);
	gl_Position = matrices[PROJECTION] * matrices[VIEW] * worldSpacePos;

	fragPos = worldSpacePos.xyz;
	color = vec4((normal.xyz+1)*0.5,1.0);
}


