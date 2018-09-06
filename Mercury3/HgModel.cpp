#include <HgModel.h>
#include <stdio.h>
#include <stdlib.h>
#include <HgVbo.h>

#include "oglDisplay.h"

#include <ini.h>
#include <IniLoader.h>
#include <StringConversions.h>

typedef struct header {
	uint32_t vertex_count, index_count;
} header;


static model_data LoadModel(const char* filename) {
	header head;
	model_data r;
	r.vertices = NULL;
	r.indices = NULL;

	vbo_layout_vnut* buffer1 = NULL;
	uint16_t* buffer2 = NULL;

	FILE* f = NULL;
	errno_t err = fopen_s(&f, filename, "rb");
	if (err != 0) {
		fprintf(stderr, "Unable to open file \"%s\"\n", filename);
		return r;
	}

	size_t read = fread(&head, sizeof(head), 1, f);
	if (read != 1) {
		fprintf(stderr, "Unable to read file header for \"%s\"\n", filename);
		fclose(f);
		return r;
	}

	if (head.vertex_count > 65535) {
		fprintf(stderr, "Too many vertices for \"%s\"\n", filename);
		fclose(f);
		return r;
	}

	if (head.index_count > 1000000) {
		fprintf(stderr, "Too many indices for \"%s\"\n", filename);
		fclose(f);
		return r;
	}

	buffer1 = (vbo_layout_vnut*)malloc(sizeof(*buffer1)*head.vertex_count);
	buffer2 = (uint16_t*)malloc(sizeof(*buffer2)*head.index_count);

	read = fread(buffer1, sizeof(*buffer1), head.vertex_count, f);
	if (read != head.vertex_count) {
		fprintf(stderr, "Error, %d vertices expected, read %zd", head.vertex_count, read);
		free(buffer1);
		free(buffer2);
		fclose(f);
		return r;
	}

	read = fread(buffer2, sizeof(*buffer2), head.index_count, f);
	if (read != head.index_count) {
		fprintf(stderr, "Error, %d indices expected, read %zd", head.index_count, read);
		free(buffer1);
		free(buffer2);
		fclose(f);
		return r;
	}

	fclose(f);

	r.vertices = buffer1;
	r.indices = buffer2;
	r.vertex_count = head.vertex_count;
	r.index_count = head.index_count;

	return std::move(r);
}

static std::shared_ptr<RenderData> init_render_data() {
	auto rd = OGLRenderData::Create();
//	rd->baseRender.renderFunc = model_render;
	return rd;
}

static void updateClbk(HgElement* e, uint32_t tdelta) {
	//	printf("cube\n");
}

model_data::model_data()
	:vertices(nullptr), indices(nullptr), element(nullptr)
{

}
model_data::~model_data() {
	if (vertices) free(vertices);
	if (indices) free(indices);

	vertices = nullptr;
	indices = nullptr;
}

model_data::model_data(model_data && other) {
	memcpy(this, &other, sizeof(model_data));
	other.vertices = nullptr;
	other.indices = nullptr;
}

model_data& model_data::operator=(model_data && other) {
	memcpy(this, &other, sizeof(model_data));
	other.vertices = nullptr;
	other.indices = nullptr;
	return *this;
}

static void destroy(HgElement* e) {
	e->destroy();
}

static void* change_to_model(HgElement* element) {
//	element->vptr_idx = VTABLE_INDEX;

	//create an instance of the render data for all triangles to share
	element->setRenderData( init_render_data() ); //this needs to be per model instance if the model is animated
	return nullptr;
}

static void render(RenderData* rd) {
	//Special render call, uses uint16_t as indices rather than uint8_t that the rest of the engine uses
	//OGLRenderData *d = (OGLRenderData*)rd;

	rd->hgVbo()->use();
//	d->colorVbo->use();

	rd->indexVbo()->use();

	setRenderAttributes(rd->blendMode, rd->renderFlags);

	rd->indexVbo()->draw(rd);
	//	draw_index_vbo(d->indexVbo, d->vbo_offset);
}

int8_t model_data::load(HgElement* element, const char* filename) {
	change_to_model(element);

	OGLRenderData* rd = (OGLRenderData*)element->renderData();

	element->flags.destroy = true;
	//SET_FLAG(element, HGE_DESTROY); //clear when we make it to the end

	model_data mdl( LoadModel(filename) );
	if (mdl.vertices == NULL || mdl.indices == NULL) return -1;

	/*
	for (int i = 0; i < mdl.vertex_count; i++) {
		float x = mdl.vertices[i].tan.x;
		float y = mdl.vertices[i].tan.y;
		float z = mdl.vertices[i].tan.z;
		float w = mdl.vertices[i].tan.w;
		printf("%f %f %f %f\n",x, y, z, w);
	}
*/
	rd->hgVbo( staticVboVNUT );
	rd->vertex_count = mdl.vertex_count;
	rd->index_count = mdl.index_count;
	rd->vbo_offset = staticVboVNUT->add_data(mdl.vertices, rd->vertex_count);
	//free(mdl.vertices);

//	mrd->index_count = mdl.index_count;
	rd->indexVbo( HgVbo::Create<uint16_t>() );
	rd->index_offset = rd->indexVbo()->add_data(mdl.indices, mdl.index_count);

	rd->renderFunction = render;

	element->flags.destroy = false;
	//CLEAR_FLAG(element, HGE_DESTROY);

	return 0;
}

bool model_data::load_ini(HgElement* element, std::string filename) {
	IniLoader::Contents contents = IniLoader::parse(filename);
	return load_ini(element, contents);
}

bool model_data::load_ini(HgElement* element, const IniLoader::Contents& contents) {
	change_to_model(element);

	float scale = 1;
	vector3 origin;
	vector3 position;
	quaternion orientation;

	StringConverters::convertValue(contents.getValue("model","scale"), scale);
	StringConverters::convertValue(contents.getValue("model","origin"), origin);
	StringConverters::convertValue(contents.getValue("model","position"), position);
	StringConverters::convertValue(contents.getValue("model","orientation"), orientation);

	const std::string& modelFilename = contents.getValue("model", "file");

	const std::string& vertexShader = contents.getValue("model", "vertexshader");
	const std::string& fragmentShader = contents.getValue("model", "fragmentshader");

	const std::string& diffuseTexture = contents.getValue("material", "diffusetexture");
	const std::string& specularTexture = contents.getValue("material", "speculartexture");
	const std::string& normalTexture = contents.getValue("material", "normaltexture");

	if (model_data::load(element, modelFilename.c_str()) < 0) return false;

	element->scale(scale);
	element->origin(origin.scale(1.0f/scale));
	element->position(position);
	element->orientation(orientation);

	if (!vertexShader.empty() && !fragmentShader.empty())
		element->m_renderData->shader = HgShader::acquire(vertexShader.c_str(), fragmentShader.c_str());

	if (!diffuseTexture.empty())
	{
		auto tmp = HgTexture::acquire(diffuseTexture, HgTexture::DIFFUSE);
		if (tmp != nullptr) {
			element->m_extendedData->textures.push_back(std::move(tmp));
			element->flags.update_textures = true;
			//SET_FLAG(element, HGE_UPDATE_TEXTURES);
		}
	}

	if (!specularTexture.empty())
	{
		auto tmp = HgTexture::acquire(specularTexture, HgTexture::SPECULAR);
		if (tmp != nullptr) {
			element->m_extendedData->textures.push_back(std::move(tmp));
			element->flags.update_textures = true;
			//SET_FLAG(element, HGE_UPDATE_TEXTURES);
		}
	}

	if (!normalTexture.empty())
	{
		auto tmp = HgTexture::acquire(normalTexture, HgTexture::NORMAL);
		if (tmp != nullptr) {
			element->m_extendedData->textures.push_back(std::move(tmp));
			element->flags.update_textures = true;
			//SET_FLAG(element, HGE_UPDATE_TEXTURES);
		}
	}

	return true;
}

/*
void model_create(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_model(element);
}
*/

REGISTER_LINKTIME(hgmodel,change_to_model);