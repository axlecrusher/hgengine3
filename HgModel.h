#pragma once

#include <HgVbo.h>
#include <HgElement.h>

typedef struct model_data {
	vbo_layout_vnu* vertices;
	uint16_t* indices;
	uint32_t vertex_count;
	uint32_t index_count;
} model_data;

//model_data LoadModel(const char* filename);

int8_t model_load(HgElement* element, const char* filename);

//void change_to_model(HgElement* element);
//void model_create(HgElement* element);