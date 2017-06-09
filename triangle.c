#include <triangle.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#define VTABLE_INDEX 1

static float vv[9] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.0f, 0.5f, 0.0f
};

static float colours[] = {
	1.0f, 0.0f,  0.0f,
	0.0f, 1.0f,  0.0f,
	0.0f, 0.0f,  1.0f
};

extern HgElement* render_thing;

/*
void gen_triangle(vertices* v) {
	static float vv[9] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f
	};

	v->points.f = (float*)malloc(9 * sizeof(float));
	memcpy(v->points.f, vv, 9 * sizeof(float));
	v->f_size = 9;
}
*/
static void setup_ogl(OGLRenderData* rd) {
	vertices points;
	points.points.array = vv;
	points.size = 3;
//	gen_triangle(&points);

	GLuint* vbo = calloc(2, sizeof(GLuint));
	vbo[0] = hgOglVbo(points);
//	free(points.points.array);

	//colors
	glGenBuffers(1, vbo+1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colours, GL_STATIC_DRAW);

	//vao is used to group GL state to supply vertex data
	//once you have all the state set, you only need to bind the vao to draw again later
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0); //enable access to attribute
	glEnableVertexAttribArray(1);

	rd->vao = vao;
	rd->vbo.id = vbo;
	rd->vbo.count = 2;
}

static void updateClbk(struct HgElement* e, uint32_t tdelta) {
	//	printf("cube\n");
}

static void destroy(struct HgElement* e) {

	//	printf("cube\n");
}

static HgElement_vtable vtable = {
	.destroy = NULL,
	.updateFunc = NULL
};

static void triangle_render(RenderData* rd) {
	OGLRenderData *d = (OGLRenderData*)rd;
	if (d->vbo.count == 0) {
		setup_ogl(d);
	}

	glBindVertexArray(d->vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
//	glBindVertexArray(0);
}

//instanced render data
static OGLRenderData* trd = NULL;

OGLRenderData* triangle_init_render_data() {
	if (trd == NULL) {
		trd = calloc(1, sizeof(*trd));
		trd->baseRender.renderFunc = triangle_render;
		trd->baseRender.shader = HGShader_acquire("test_vertex.glsl", "test_frag.glsl");
	}
	return trd;
}

void change_to_triangle(HgElement* element) {
//	element->vptr = &vtable;
	element->vptr_idx = 0;

	//create an instance of the render data for all triangles to share
	element->m_renderData = (RenderData*)triangle_init_render_data();
}

void shape_create_triangle(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_triangle(element);
}
