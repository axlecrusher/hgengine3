#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>

HgVboMemory staticVbo;

static vbo_layout1* resize(HgVboMemory* vbo_mem, uint32_t count) {
	vbo_layout1* buf = realloc(vbo_mem->buffer, count * sizeof* buf);
	assert(buf != NULL);
	vbo_mem->buffer = buf;

	return buf;
}

uint32_t hgvbo_add_data(HgVboMemory* vbo_mem, vertex* vertices, color* color, uint16_t count) {
	vbo_layout1* buf = resize(vbo_mem, vbo_mem->size + count);
	buf = buf + vbo_mem->size;

	for (uint16_t i = 0; i < count; ++i) {
		buf[i].v = vertices[i];
		buf[i].c = color[i];
	}

	uint32_t offset = vbo_mem->size;
	vbo_mem->size += count;
	vbo_mem->needsUpdate = 1;

	return offset;
}

void hgvbo_sendToOGL(HgVboMemory* vbo_mem) {
	if (vbo_mem->vbo_id == 0) glGenBuffers(1, &vbo_mem->vbo_id);
	if (vbo_mem->vao_id == 0) glGenVertexArrays(1, &vbo_mem->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_mem->vbo_id);
	glBufferData(GL_ARRAY_BUFFER, vbo_mem->size * sizeof(*(vbo_mem->buffer)), vbo_mem->buffer, GL_STATIC_DRAW);

	glBindVertexArray(vbo_mem->vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_mem->vbo_id);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO
	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(*(vbo_mem->buffer)), NULL);
	glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(*(vbo_mem->buffer)), (void*)sizeof(vbo_mem->buffer->v));

	glEnableVertexAttribArray(L_VERTEX); //enable access to attribute
	glEnableVertexAttribArray(L_COLOR);

	vbo_mem->needsUpdate = 0;
}


void hgvbo_use(HgVboMemory* vbo) {
	if (vbo->needsUpdate > 0) {
		hgvbo_sendToOGL(vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo->vbo_id);
	glBindVertexArray(vbo->vao_id);
}