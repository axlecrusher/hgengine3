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