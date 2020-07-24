#pragma once

#include <vector>
#include <HgVbo.h>

namespace MeshMath
{
	//computes tanges for a triangle
	void computeTangentsTriangle(const vbo_layout_vnut* vertices, uint32_t* indices, vector3f* tangent, vector3f* bitangent);

	template<typename T>
	inline void computeTangents(vbo_layout_vnut* vertices, uint32_t vertice_count, T* indices, uint32_t indice_count)
	{
		std::vector<vector3f> tangents;
		std::vector<vector3f> bitangents;

		tangents.resize(vertice_count);
		bitangents.resize(vertice_count);

		for (uint32_t i = 0; i < indice_count; i += 3)
		{
			uint32_t idx[3] = { indices[i], indices[i + 1], indices[i + 2] };
			computeTangentsTriangle(vertices, idx, tangents.data(), bitangents.data());
		}

		for (uint32_t i = 0; i < vertice_count; ++i)
		{
			const auto& t = tangents[i];
			const auto& b = bitangents[i];
			const auto n = vector3f(vertices[i].n.x, vertices[i].n.y, vertices[i].n.z);

			const auto tn = Reject(t, n).normal();

			const auto w = t.cross(b).dot(n) > 0.0f ? 1.0f : -1.0f; //handiness multiplier for building bitangent
			//const auto bt = n.cross(tn).scale(w); //this is how to build the tangent

			vertices[i].tan = tangent{ tn.x(), tn.y(), tn.z(), w };
		}
	}

	//calculate unit direction vector from a to b
	inline vertex3f unitDirection(const vector3f& a, const vector3f& b)
	{
		const auto v = b - a;
		return (v / v.magnitude());
	}

	//compute the number of vertices required for a polyboard following a path of N vertices
	inline constexpr int32_t computePolyboardVertexCount(int32_t N) { return N*2; }

	//compute the number of indices required for a polyboard following a path of N vertices
	inline constexpr int32_t computePolyboardIndexCount(int32_t N) { return (N - 1) * 6; }

	/* generate a polyboard mesh following a path constructed with points in vertexArray.
	outputVertexArray must be twice the size of input vertexArray
	outputIndexArray array size must equal (vertexCount-1)*6. use computePolyboardIndexCount() to calculate.
	Indices are arranged as a triangle list.
	*/
	void generatePolyboard(const vertex3f* vertexArray, int32_t vertexCount, const vector3f& camera, float r, vertex3f* outputVertexArray, uint32_t* outputIndexArray);
}