#pragma once

#include <HgVbo.h>
#include <HgElement.h>

#include <string>
#include <IniLoader.h>

class model_data {
public:
	model_data() :element(nullptr) {}
	~model_data() = default;

	std::shared_ptr<vbo_layout_vnut[]> vertices;
	std::shared_ptr<uint16_t[]> indices16;
	std::shared_ptr<uint32_t[]> indices32;
	uint32_t vertex_count;
	uint32_t index_count;

	HgElement *element;


	static int8_t load(HgElement* element, const char* filename);

	static bool load_ini(HgElement* element, std::string filename);
	static bool load_ini(HgElement* element, const IniLoader::Contents& ini);

private:



};

//model_data LoadModel(const char* filename);
//class HgElement;


//void change_to_model(HgElement* element);
//void model_create(HgElement* element);