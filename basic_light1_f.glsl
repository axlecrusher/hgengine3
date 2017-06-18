#version 420
#extension GL_ARB_explicit_uniform_location : enable

//uniform vec4 inputColour;
out vec4 fragColour;

in vec4 color;
in vec3 frag_normal;
//in vec3 eye_vector;

in vec3 position;
//in vec3 eye_pos;

layout(location = 7) uniform vec3 cam_position;

vec3 lightDirection = normalize(vec3(0,0,1));
vec3 frag_color = vec3(0.8,0,0);
vec3 specular = vec3(0.5);

void main() {
	vec3 normal = normalize(frag_normal);
	vec3 eye = normalize(cam_position - position);
	vec3 spec = vec3(0.0);

	float intensity = max(dot(normal,lightDirection),0);

	if (intensity > 0.0) {
		vec3 h = normalize(lightDirection + eye);
		float intSpec = max(dot(h,normal), 0.0);
		spec = specular * pow(intSpec,50);
	}

  fragColour = vec4(max(intensity * color.rgb + spec, vec3(0.15)* color.rgb),1);
//  fragColour = vec4(spec,1);
}