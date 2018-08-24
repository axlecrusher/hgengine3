#version 420

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 normal;
layout(location = 3) in vec4 v_color;

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
	vec3 worldSpaceNormal = normalize(mat3(matrices[MODEL]) * normal.xyz);
	fragNormal = worldSpaceNormal;

	eyePos = matrices[VIEW][3].xyz*-1.0; //pull eye position from transform matrix

	vec4 worldSpacePos =  matrices[MODEL] * vec4(vertex,1);
	fragPos = worldSpacePos.xyz;
	gl_Position = matrices[PROJECTION] * matrices[VIEW] * worldSpacePos;
	color = vec4((normal.xyz+1)*0.5,1.0);
}


