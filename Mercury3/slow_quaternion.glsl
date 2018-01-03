vec4 quat_mult(vec4 q1, vec4 q2) {
	vec4 q;
	q.w = (q1.w*q2.w)-(q1.x*q2.x)-(q1.y*q2.y)-(q1.z*q2.z);
	q.x = (q1.w*q2.x)+(q1.x*q2.w)-(q1.y*q2.z)+(q1.z*q2.y);
	q.y = (q1.w*q2.y)+(q1.x*q2.z)+(q1.y*q2.w)-(q1.z*q2.x);
	q.z = (q1.w*q2.z)-(q1.x*q2.y)+(q1.y*q2.x)+(q1.z*q2.w);
	return q;
}

vec3 quat_v_rot(vec3 v) {
	vec4 q_conj = vec4(-q.x,-q.y,-q.z,q.w);
	vec4 v_quat = vec4(v,1.0);

	vec4 qtmp = quat_mult(v_quat,q);
	v_quat = quat_mult(q_conj,qtmp);

	return v_quat.xyz;
}