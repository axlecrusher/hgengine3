#include <stdint.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <assert.h>

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

static float colours[] = {
	1.0f, 0.0f,  0.0f, //0
	0.0f, 1.0f,  0.0f, //1
	0.0f, 0.0f,  1.0f, //2
	1.0f, 0.0f,  0.0f, //3
	0.0f, 1.0f,  0.0f, //4
	0.0f, 0.0f,  1.0f, //5
	1.0f, 0.0f,  0.0f, //6
	0.0f, 1.0f,  0.0f //7
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

	GLuint* vbo = calloc(3, sizeof(GLuint));
	vbo[0] = hgOglVbo(points);

	//colors
	glGenBuffers(1, vbo + 1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(*colours), colours, GL_STATIC_DRAW);

	//indices
	glGenBuffers(1, vbo + 2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(*indices), indices, GL_STATIC_DRAW);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

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
	rd->vbo.count = 3;
}

//instanced render data
static OGLRenderData *crd = NULL;

void cube_render(HgElement* element) {
	OGLRenderData *d = (OGLRenderData*)element->m_renderData;
	if (d->vbo.count == 0) {
		cube_setup_ogl(d);
	}

//	if (d->oglRender.shader_program > 0) useShaderProgram(d->oglRender.shader_program);

	//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
	//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated
	setGlobalUniforms();
	setLocalUniforms(&element->rotation, &element->position, element->scale);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->vbo.id[2]);
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
	element->vptr = &vtable;
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
