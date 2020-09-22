#version 420

//uniform vec4 inputColour;
out vec4 fragColour;
in vec4 color;
in vec4 fragCoord;
in vec3 camPos;

vec4 grid_color = vec4(132/255,191/255,1,1);
void main() {
  // Pick a coordinate to visualize in a grid
  vec2 coord = fragCoord.xz;

  float dist = abs(distance(coord, camPos.xz))/15.0;

  // Compute anti-aliased world-space grid lines
//  vec2 grid = (abs(mod(coord,vec2(2))-1)/2) / fwidth(coord);
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
  float line = min(grid.x, grid.y);
  float glow = min(grid.x, grid.y)*0.25; //fat line

//  line -= dist;

  // Just visualize the grid lines directly
  float thing1 = min(line + dist, 1.0);
  float thing2 = min(glow + dist, 1.0);
  vec3 lineColor = vec3(1.0 - thing1);
  vec3 glowColor = clamp(vec3(132/255.0,191/255.0,1.0)-thing2,0,1);

  vec3 c = mix(glowColor+lineColor,vec3(0), thing2);

//  gl_FragColor = vec4(glowColor+lineColor,1-min(thing2,1.0));
  gl_FragColor = vec4(glowColor+lineColor,1);
//gl_FragColor = vec4(c,1);



/*
	vec4 coord = fwidth(fragCoord);

	float v_mod = mod(fragCoord.x+0.25, 1.0);
	float r_sin = clamp((sin(v_mod*6.2831855)*75.0)-74.0,0.0,1.0);

	float v_mod2 = mod(fragCoord.z+0.25, 1.0);
	r_sin += clamp((sin(v_mod2*6.2831855)*75.0)-74.0,0.0,1.0);
	r_sin = clamp(r_sin,0.0,1.0);

//	smoothstep(0.45-delta, 0.45, dist);
//	r_sin = smoothstep(0.0,1.0, r_sin);

//	r_sin = fwidth(r_sin);
	//0,0.1,0.11,0.12,0.22

	vec4 grid_color = vec4(132/255.0,191/255.0,1,1*r_sin);
	fragColour = grid_color;
//  fragColour = vec4(coord.xyz,1);
*/
}