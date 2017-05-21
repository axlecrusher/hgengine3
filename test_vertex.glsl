#version 420
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 v_color;

//uniforms have their own locations separate from attributes
layout(location = 1) uniform vec4 rotation;
layout(location = 3) uniform vec3 translation;
layout(location = 5) uniform mat4 projection;

layout(location = 6) uniform vec4 cam_rot;
layout(location = 7) uniform vec3 cam_position;

out vec3 color;

vec4 quat_mult(vec4 q1, vec4 q2) {
	vec4 q;
	q.w = (q1.w*q2.w)-(q1.x*q2.x)-(q1.y*q2.y)-(q1.z*q2.z);
	q.x = (q1.w*q2.x)+(q1.x*q2.w)-(q1.y*q2.z)+(q1.z*q2.y);
	q.y = (q1.w*q2.y)+(q1.x*q2.z)+(q1.y*q2.w)-(q1.z*q2.x);
	q.z = (q1.w*q2.z)-(q1.x*q2.y)+(q1.y*q2.x)+(q1.z*q2.w);
	return q;
}


vec3 point_quat_rotate(vec3 v, vec4 quaternion)
{ 
  return v + 2.0 * cross(quaternion.xyz, cross(quaternion.xyz, v) + quaternion.w * v);
}

void main() {
	//roatate around vector first if rotation needs to be around an arbitrary vector

	vec3 v1 = point_quat_rotate(vertex,rotation) + translation + cam_position;
	vec4 v2 = vec4(point_quat_rotate(v1,cam_rot), 1.0);

	gl_Position = projection * v2;
	color = v_color;
}


