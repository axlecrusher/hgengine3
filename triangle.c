#include <triangle.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

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

typedef struct triangle_render_data {
	OGLRenderData oglRender;
	uint8_t vbo_created;
} triangle_render_data;

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
	rd->vbo = vbo;
	rd->vbo_size = 2;
}

//instanced render data
static triangle_render_data *trd = NULL;

void triangle_render(HgElement* element) {
	triangle_render_data *d = (triangle_render_data*)element->m_renderData;
	if (d->vbo_created == 0) {
		setup_ogl(&d->oglRender);
		d->vbo_created = 1;
	}

	//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
	//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated
	setGlobalUniforms();
	setLocalUniforms(&element->rotation, &element->position, element->scale);

	glBindVertexArray(d->oglRender.vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
//	glBindVertexArray(0);
}

void change_to_triangle(HgElement* element) {
	element->vptr = NULL;
	//create an instance of the render data for all triangles to share
	if (trd == NULL) {
		trd = calloc(1, sizeof(triangle_render_data));
		trd->oglRender.baseRender.renderFunc = triangle_render;

		trd->oglRender.baseRender.shader = HGShader_aquire("test_vertex.glsl", "test_frag.glsl");
		VCALL(trd->oglRender.baseRender.shader, load);
	}
	element->m_renderData = (RenderData*)trd;
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
