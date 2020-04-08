#version 420

uniform sampler2D diffuseTex;

out vec4 fragColor;
in vec2 frag_uv;

void main() {
	float c = texture(diffuseTex, frag_uv).r;
	fragColor = vec4(c,c,c,c);
}