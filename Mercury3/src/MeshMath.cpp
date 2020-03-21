#include "MeshMath.h"

namespace MeshMath
{

	template<typename T>
	float floatFromNormalInt(T x)
	{
		const float max = std::numeric_limits<T>::max();
		return x / max;
	}

	//computes tangents for a triangle as defined by 3 consecutive indices
	void computeTangentsTriangle(const vbo_layout_vnut* vertices, uint32_t* indices, vector3f* tangent, vector3f* bitangent)
	{
		const auto& v0 = vertices[indices[0]];
		const auto& v1 = vertices[indices[1]];
		const auto& v2 = vertices[indices[2]];

		//printf("%d %d\n", v0.uv.x, v0.uv.y);

		const auto e1 = v1.v.object - v0.v.object;
		const auto e2 = v2.v.object - v0.v.object;

		const auto x1 = floatFromNormalInt(v1.uv.x.value) - floatFromNormalInt(v0.uv.x.value);
		const auto x2 = floatFromNormalInt(v2.uv.x.value) - floatFromNormalInt(v0.uv.x.value);
		const auto y1 = floatFromNormalInt(v1.uv.y.value) - floatFromNormalInt(v0.uv.y.value);
		const auto y2 = floatFromNormalInt(v2.uv.y.value) - floatFromNormalInt(v0.uv.y.value);

		const float r = 1.0f / (x1 * y2 - x2 * y1);
		const vector3f t = (e1.scale(y2) - e2.scale(y1)).scale(r);
		const vector3f b = (e2.scale(x1) - e1.scale(x2)).scale(r);

		tangent[indices[0]] += t;
		tangent[indices[1]] += t;
		tangent[indices[2]] += t;

		bitangent[indices[0]] += b;
		bitangent[indices[1]] += b;
		bitangent[indices[2]] += b;
	}

}