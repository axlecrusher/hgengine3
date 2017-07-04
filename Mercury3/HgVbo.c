#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>

HgVboMemory staticVbo;
HgVboMemory staticVboVNU;

static HgVboMemory* _currentVbo;

static uint16_t struct_size(VBO_TYPE type) {
	if (type == VBO_VC) return sizeof(vbo_layout_vc);
	if (type == VBO_VNU) return sizeof(vbo_layout_vnu);
	if (type == VBO_INDEX8) return sizeof(uint8_t);
	if (type == VBO_INDEX16) return sizeof(uint16_t);

	fprintf(stderr, "Unknown vbo type:%d\n", type);
	assert(!"Unknown vbo type");
	return 0;
}

void hgvbo_init(HgVboMemory* vbo_mem, VBO_TYPE type) {
	vbo_mem->type = type;
	vbo_mem->size = struct_size(type);
}

static void* resize(HgVboMemory* vbo_mem, uint32_t count, uint32_t size) {
	void* buf = realloc(vbo_mem->buffer, count * size);
	assert(buf != NULL);
	vbo_mem->buffer = buf;

	return buf;
}

void hgvbo_clear(HgVboMemory* vbo_mem) {
	free(vbo_mem->buffer);
	vbo_mem->buffer = NULL;
	vbo_mem->count = 0;
}

void hgvbo_destroy(HgVboMemory* vbo) {
	glDeleteBuffers(1, vbo->vbo_id);
	glDeleteBuffers(1, vbo->vao_id);
	free(vbo->buffer);
	vbo->buffer = NULL;
	vbo->count = 0;
}

uint32_t hgvbo_add_data_vc(HgVboMemory* vbo_mem, vertex* vertices, color* color, uint16_t count) {
	vbo_layout_vc* buf = resize(vbo_mem, vbo_mem->count + count, sizeof(*buf));
	buf = buf + vbo_mem->count;

	for (uint16_t i = 0; i < count; ++i) {
		buf[i].v = vertices[i];
		buf[i].c = color[i];
	}

	uint32_t offset = vbo_mem->count;
	vbo_mem->count += count;
	vbo_mem->needsUpdate = 1;

	return offset;
}

uint32_t hgvbo_add_data_vnu_raw(HgVboMemory* vbo_mem, vbo_layout_vnu* data, uint16_t count) {
	vbo_layout_vnu* buf = resize(vbo_mem, vbo_mem->count + count, sizeof(*buf));
	buf = buf + vbo_mem->count;

	for (uint16_t i = 0; i < count; ++i) {
		buf[i]= data[i];
	}

	uint32_t offset = vbo_mem->count;
	vbo_mem->count += count;
	vbo_mem->needsUpdate = 1;

	return offset;
}

uint32_t hgvbo_add_data_raw(HgVboMemory* vbo_mem, void* data, uint16_t count) {
	uint8_t* buf = resize(vbo_mem, vbo_mem->count + count, vbo_mem->size);
	buf = buf + (vbo_mem->count * vbo_mem->size);

	memcpy(buf, data, count*vbo_mem->size);

	uint32_t offset = vbo_mem->count;
	vbo_mem->count += count;
	vbo_mem->needsUpdate = 1;

	return offset;
}

static void hgvbo_sendogl(HgVboMemory* vbo_mem) {
	if (vbo_mem->vbo_id == 0) glGenBuffers(1, &vbo_mem->vbo_id);
	if (vbo_mem->vao_id == 0) glGenVertexArrays(1, &vbo_mem->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_mem->vbo_id);
	glBufferData(GL_ARRAY_BUFFER, vbo_mem->count * vbo_mem->size, vbo_mem->buffer, GL_STATIC_DRAW);

	glBindVertexArray(vbo_mem->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_mem->vbo_id);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO

	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, vbo_mem->size, NULL);
	glEnableVertexAttribArray(L_VERTEX); //enable access to attribute

	if (vbo_mem->type == VBO_VC) {
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, vbo_mem->size, (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_COLOR);
	}
	else if (vbo_mem->type == VBO_VN) {
		glVertexAttribPointer(L_NORMAL, 4, GL_FLOAT, GL_FALSE, vbo_mem->size, (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
	}
	else if (vbo_mem->type == VBO_VNU) {
		glVertexAttribPointer(L_NORMAL, 4, GL_FLOAT, GL_FALSE, vbo_mem->size, (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_FALSE, vbo_mem->size, (void*)(sizeof(vertex)+sizeof(normal)));
		glEnableVertexAttribArray(L_UV);
	}
	else {
		fprintf(stderr, "Unknown vbo type:%d\n", vbo_mem->type);
		assert(!"Unknown vbo type");
	}

	vbo_mem->needsUpdate = 0;
}


void hgvbo_use(HgVboMemory* vbo) {
	if (_currentVbo == vbo) return;
	_currentVbo = vbo;

	if (vbo->needsUpdate > 0) {
		hgvbo_sendogl(vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo->vbo_id); //is this needed or does the vao_id do this for us?
	glBindVertexArray(vbo->vao_id);
}

void use_index16vbo(HgVboMemory* vbo) {
	if (vbo->vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		vbo->vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->vbo_id);

	if (vbo->needsUpdate > 0) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vbo->count * vbo->size, vbo->buffer, GL_STATIC_DRAW);
		vbo->needsUpdate = 0;
	}
}