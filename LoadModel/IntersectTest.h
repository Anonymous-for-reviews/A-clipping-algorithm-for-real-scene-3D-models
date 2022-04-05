#pragma once

struct tri3d
{
	float v0[3];
	float v1[3];
	float v2[3];
};

class IntersectTest {
	float vector_dot(float v0[3], float v1[3])
	{
		return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
	}



	void vector_minus(float a[3], float b[3], float res[3])
	{
		res[0] = a[0] - b[0];
		res[1] = a[1] - b[1];
		res[2] = a[2] - b[2];
	}



	void vector_cross(float a[3], float b[3], float res[3])
	{
		res[0] = a[1] * b[2] - a[2] * b[1];
		res[1] = a[2] * b[0] - a[0] * b[2];
		res[2] = a[0] * b[1] - a[1] * b[0];
	}



public:	bool LineTriangleIntersect(float start[3], float end[3], tri3d tri, float intersection[3])
	{
		const float epsilon = 0.000001f;
		float e1[3], e2[3], p[3], s[3], q[3];
		float t, u, v, tmp;
		float direction[3];
		vector_minus(end, start, direction);
		vector_minus(tri.v1, tri.v0, e1);
		vector_minus(tri.v2, tri.v0, e2);
		vector_cross(direction, e2, p);
		tmp = vector_dot(p, e1);
		if (tmp > -epsilon && tmp < epsilon)
			return false;
		tmp = 1.0f / tmp;
		vector_minus(start, tri.v0, s);
		u = tmp * vector_dot(s, p);
		if (u < 0.0 || u > 1.0)
			return false;
		vector_cross(s, e1, q);
		v = tmp * vector_dot(direction, q);
		if (v < 0.0 || v > 1.0)
			return false;
		if (u + v > 1.0)
			return false;
		t = tmp * vector_dot(e2, q);
		if (t < 0.0 || t > 1.0)
			return false;
		intersection[0] = start[0] + t * direction[0];
		intersection[1] = start[1] + t * direction[1];
		intersection[2] = start[2] + t * direction[2];
		return true;
	}
};
