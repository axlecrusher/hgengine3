#version 420

out vec4 fragColor;

in vec4 color;
in vec3 viewSpaceNormal;
in vec3 viewSpaceFragPos;
in vec3 viewSpaceLightDir;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[4];
uniform samplerBuffer bufferObject1;

//All computations are in view space
vec3 frag_color = vec3(0.8,0,0);
float specular = 0.5;
float shininess = 35.0;

void main() {
	vec3 normal = normalize(viewSpaceNormal);
	vec3 fragDir = normalize(-viewSpaceFragPos); //eyePos - fragPos
//	viewDir = vec3(0,0,1);
	float spec = 0.0;

	float intensity = max(dot(viewSpaceLightDir,normal),0);

	if (intensity > 0.0) {
		vec3 halfDir = normalize(viewSpaceLightDir + fragDir);
		float specAngle = max(dot(halfDir,normal), 0.0);
		spec = specular * pow(specAngle,shininess);
	}

  fragColor = vec4(max(intensity * color.rgb + spec, vec3(0.15)* color.rgb),1);
}