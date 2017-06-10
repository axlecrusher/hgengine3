#include <stdint.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <assert.h>

#include <memory.h>

#define VTABLE_INDEX 2

static float cube_verts[] = {
	0.5f, 0.5, 0.5,		//0
	0.5f, -0.5, 0.5,	//1
	-0.5f, -0.5, 0.5,	//2
	-0.5f, 0.5, 0.5,	//3
	0.5f, 0.5, -0.5,	//4
	0.5f, -0.5, -0.5,	//5
	-0.5f, -0.5, -0.5,	//6
	-0.5f, 0.5, -0.5	//7
};

//rgba
static color colors[] = {
	255, 0, 0, 255, //0
	0, 255, 0, 255, //1
	0, 0, 255, 255, //2
	255, 0, 0, 255, //3
	0, 255, 0, 255, //4
	0, 0, 255, 255, //5
	255, 0, 0, 255, //6
	0, 255, 0, 255 //7
};


static uint8_t indices[] = {
	2,1,0,0,3,2, //front
	5,6,7,7,4,5, //back
	6,5,1,1,2,6, //bottom
	3,0,4,4,7,3, //top
	6,2,3,3,7,6, //R side
	1,5,4,4,0,1  //L side
};

static void cube_setup_ogl(OGLRenderData* rd) {
	vertices points;
	points.points.array = cube_verts;
	points.size = 8;

	uint32_t vbo_size = (points.size * sizeof(*points.points.v)) //vertices
		+ (8 * sizeof(*colors));

	uint8_t* buffer = malloc(vbo_size);
	uint8_t* fptr = buffer;
	uint8_t* cptr = buffer + (points.size * sizeof(*points.points.v));

	memcpy(fptr, cube_verts, (points.size * sizeof(*points.points.v)));
	memcpy(cptr, colors, (8 * sizeof(*colors)));

	GLuint* vbo = malloc(2*sizeof(*vbo));

	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

	glBufferData(GL_ARRAY_BUFFER, vbo_size, buffer, GL_STATIC_DRAW);

	free(buffer);

	//indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(*indices), indices, GL_STATIC_DRAW);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

	//vao is used to group GL state to supply vertex data
	//once you have all the state set, you only need to bind the vao to draw again later
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO
	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(L_COLOR, sizeof(*colors), GL_UNSIGNED_BYTE, GL_TRUE, 0, cptr-fptr);

	glEnableVertexAttribArray(0); //enable access to attribute
	glEnableVertexAttribArray(1);

	rd->vao = vao;
	rd->vbo.id = vbo;
	rd->vbo.count = 2;
}

//instanced render data
static OGLRenderData *crd = NULL;

static void cube_render(RenderData* rd) {
	OGLRenderData *d = (OGLRenderData*)rd;
	if (d->vbo.count == 0) {
		cube_setup_ogl(d);
	}

//	if (d->oglRender.shader_program > 0) useShaderProgram(d->oglRender.shader_program);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->vbo.id[1]);
	glBindVertexArray(d->vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
}

static void updateClbk(struct HgElement* e, uint32_t tdelta) {
//	printf("cube\n");
}

static void destroy(struct HgElement* e) {

	//	printf("cube\n");
}

static HgElement_vtable vtable = {
	.destroy = destroy,
	.updateFunc = updateClbk
};

void change_to_cube(HgElement* element) {
	HGELEMT_VTABLES[VTABLE_INDEX] = vtable; //how to only do this once?

//	element->vptr = &vtable;
	element->vptr_idx = VTABLE_INDEX;
	//create an instance of the render data for all triangles to share
	if (crd == NULL) {
		crd = calloc(1, sizeof(*crd));
		crd->baseRender.renderFunc = cube_render;
		crd->baseRender.shader = HGShader_acquire("test_vertex.glsl", "test_frag.glsl");
	}
	element->m_renderData = (RenderData*)crd;
}

void shape_create_cube(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;
	element->scale = 1.0f;

	change_to_cube(element);
}
