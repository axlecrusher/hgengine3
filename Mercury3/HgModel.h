#pragma once

#include <HgVbo.h>
#include <HgElement.h>

#include <string>

class model_data {
public:
	model_data();
	~model_data();
	vbo_layout_vnu* vertices;
	uint16_t* indices;
	uint32_t vertex_count;
	uint32_t index_count;
};

//model_data LoadModel(const char* filename);
//class HgElement;

int8_t model_load(HgElement* element, const char* filename);
bool model_load_ini(HgElement* element,std::string filename);


//void change_to_model(HgElement* element);
//void model_create(HgElement* element);