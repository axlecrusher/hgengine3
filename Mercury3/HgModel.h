#pragma once

#include <HgVbo.h>
#include <HgEntity.h>

#include <string>
#include <IniLoader.h>

class model_data {
public:
	model_data() :entity(nullptr) {}
	~model_data() = default;

	std::shared_ptr<vbo_layout_vnut[]> vertices;
	std::shared_ptr<uint16_t[]> indices16;
	std::shared_ptr<uint32_t[]> indices32;
	uint32_t vertex_count;
	uint32_t index_count;

	HgEntity *entity;


	static int8_t load(HgEntity* entity, const char* filename);

	static bool load_ini(HgEntity* entity, std::string filename);
	static bool load_ini(HgEntity* entity, const IniLoader::Contents& ini);

private:



};

//model_data LoadModel(const char* filename);
//class HgEntity;


//void change_to_model(HgEntity* element);
//void model_create(HgEntity* element);
