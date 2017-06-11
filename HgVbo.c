#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>

HgVboMemory staticVbo;

static HgVboMemory* _currentVbo;

void hgvbo_init(HgVboMemory* vbo_mem, VBO_TYPE type) {
	vbo_mem->type = type;

	if (type == VBO_VC)	vbo_mem->size = sizeof(vbo_layout_vc);
}

static void* resize(HgVboMemory* vbo_mem, uint32_t count, uint32_t size) {
	void* buf = realloc(vbo_mem->buffer, count * size);
	assert(buf != NULL);
	vbo_mem->buffer = buf;

	return buf;
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
		glEnableVertexAttribArray(L_COLOR);
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