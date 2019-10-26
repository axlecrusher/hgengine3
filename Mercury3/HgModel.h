#pragma once

#include <HgVbo.h>
#include <HgEntity.h>

#include <string>
#include <IniLoader.h>

class model_data {
public:
	model_data() :entity(nullptr) {}
	~model_data() = default;

	const std::shared_ptr<vbo_layout_vnut[]>& getVertices() const { return m_vertices; }
	const std::shared_ptr<uint16_t[]>& getIndices16() const { return m_indices16; }
	const std::shared_ptr<uint32_t[]>& getIndices32() const { return m_indices32; }

	uint32_t getVertexCount() const { return m_vertexCount; }
	uint32_t getIndexCount() const { return m_indexCount; }

	void storeVertices(std::shared_ptr<vbo_layout_vnut[]> ptr, uint32_t c) { m_vertices = std::move(ptr); m_vertexCount = c; }
	void storeIndices(std::shared_ptr<uint16_t[]> ptr, uint32_t c) { m_indices16 = std::move(ptr);  m_indexCount = c; }
	void storeIndices(std::shared_ptr<uint32_t[]> ptr, uint32_t c) { m_indices32 = std::move(ptr);  m_indexCount = c; }

	HgEntity *entity;


	static int8_t load(HgEntity* entity, const char* filename);

	static bool load_ini(HgEntity* entity, std::string filename);
	static bool load_ini(HgEntity* entity, const IniLoader::Contents& ini);

private:
	std::shared_ptr<vbo_layout_vnut[]> m_vertices;
	std::shared_ptr<uint16_t[]> m_indices16;
	std::shared_ptr<uint32_t[]> m_indices32;

	uint32_t m_vertexCount;
	uint32_t m_indexCount;
};

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