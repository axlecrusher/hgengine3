#pragma once

#include <HgVbo.h>
#include <HgElement.h>

#include <string>

class model_data {
public:
	model_data();
	~model_data();
	vbo_layout_vnut* vertices;
	uint16_t* indices;
	uint32_t vertex_count;
	uint32_t index_count;

	HgElement *element;


	static int8_t load(HgElement* element, const char* filename);
	static bool load_ini(HgElement* element, std::string filename);

};

//model_data LoadModel(const char* filename);
//class HgElement;


//void change_to_model(HgElement* element);
//void model_create(HgElement* element);