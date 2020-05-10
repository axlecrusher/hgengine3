#include "MeshMath.h"

#include <algorithm>

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

	void generatePolyboard(const vertex3f* p, int32_t vertexCount, const vector3f& camera, float r, vertex3f* outputVertexArray, uint32_t* indexArray)
	{
		for (int32_t i = 0; i < vertexCount; i++)
		{

			const auto Z = unitDirection(p[i], camera);
			//const auto Zmag = Z.magnitude();

			const auto idxA = std::max(0, i - 1);
			const auto idxB = std::min(i + 1, vertexCount - 1);

			auto T = unitDirection(p[idxA], p[idxB]);

			const auto tcz = T.cross(Z).normal().scale(r);
			const auto G = p[i] + tcz;
			const auto H = p[i] - tcz;

			outputVertexArray[(i*2)] = G;
			outputVertexArray[(i*2)+1] = H;

		}

		int32_t vertexIdx = 0;
		int32_t idxOffset = 0;
		for (int32_t i = 0; i < vertexCount-1; i++)
		{
			//triangle list indices
			indexArray[idxOffset + 0] = 3 + vertexIdx;
			indexArray[idxOffset + 1] = 1 + vertexIdx;
			indexArray[idxOffset + 2] = 0 + vertexIdx;
			indexArray[idxOffset + 3] = 2 + vertexIdx;
			indexArray[idxOffset + 4] = 3 + vertexIdx;
			indexArray[idxOffset + 5] = 0 + vertexIdx;

			vertexIdx += 2;
			idxOffset += 6;
		}

	}

}