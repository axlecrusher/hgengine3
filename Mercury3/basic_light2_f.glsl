#version 420

out vec4 fragColor;

in vec4 color;
in vec3 fragNormal;
in vec3 eyePos;
in vec3 fragPos;

#define MODEL 0
#define PROJECTION 1
#define VIEW 2

//model, projection, and view matrices
uniform mat4 matrices[3];

//All computations are in world space
vec3 lightDir = vec3(0,0,1);
vec3 frag_color = vec3(0.8,0,0);
float specular = 0.5;
float shininess = 35.0;

void main() {
	vec3 normal = normalize(fragNormal);
	vec3 viewDir = normalize(eyePos - fragPos);
	float spec = 0.0;

	float intensity = max(dot(lightDir,normal),0);

	if (intensity > 0.0) {
		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(halfDir,normal), 0.0);
		spec = specular * pow(specAngle,shininess);
	}

  fragColor = vec4(max(intensity * color.rgb + spec, vec3(0.15)* color.rgb),1);
}