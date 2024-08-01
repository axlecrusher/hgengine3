#version 420

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 uv_coord;
layout(location = 3) in vec4 v_color;

layout(location = 5) in mat4 ModelMatrix;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2
#define PVM 3

//model, projection, and view matrices
uniform mat4 matrices[4];
//uniform samplerBuffer bufferObject1;

out vec4 color;
out vec3 viewSpaceNormal;
out vec3 viewSpaceFragPos;
out vec3 viewSpaceLightDir;
//out vec3 eyePos;

vec3 lightDir = vec3(0.0,0.00001,-1); //ever so slightly off kilter directional light

void main() {
	int bufferOffset = gl_InstanceID*4;

	mat4 MV = matrices[VIEW] * ModelMatrix;

	//Our lighting calulations expect to be in the direction of the towrds the light,
	//but it makes more sense to specify the light in world space coordiates
	//so reverse the vector.
	viewSpaceLightDir = mat3(matrices[VIEW]) * (normalize(-lightDir));

	viewSpaceNormal = normalize(mat3(MV) * normal.xyz);

	vec4 viewSpacePos = MV * vec4(vertex,1);
	gl_Position = matrices[PROJECTION] * viewSpacePos;

	viewSpaceFragPos = viewSpacePos.xyz;
	color = vec4((normal.xyz+1)*0.5,1.0);
}


