#include <HgModel.h>
#include <stdio.h>
#include <stdlib.h>
#include <HgVbo.h>

#include "oglDisplay.h"

#include <ini.h>
#include <IniLoader.h>
#include <StringConversions.h>

#include <MeshMath.h>
#include <Logging.h>

typedef struct header {
	uint32_t vertex_count, index_count;
} header;

std::unique_ptr<FILE, decltype(&fclose)> open(const char* filename) {
	std::unique_ptr<FILE, decltype(&fclose)> ret(nullptr,fclose);

	FILE* f = NULL;
	errno_t err = fopen_s(&f, filename, "rb");
	if (err != 0) {
		LOG_ERROR("Unable to open file \"%s\"", filename);
	}
	else {
		ret = std::unique_ptr<FILE, decltype(&fclose)>(f,fclose);
	}

	return ret;
}

static model_data LoadModel(const char* filename) {
	header head;
	model_data r;

	auto file = open(filename);
	if (file == nullptr) return r;

	auto f = file.get();
	size_t read = fread(&head, sizeof(head), 1, f);
	if (read != 1) {
		LOG_ERROR("Unable to read file header for \"%s\"", filename);
		return r;
	}

	//if (head.vertex_count > 65535) {
	//	LOG_ERROR("Too many vertices for \"%s\"", filename);
	//	fclose(f);
	//	return r;
	//}

	int sizeOfIndices = 0;
	void* indexBuffer = nullptr;

	std::shared_ptr<uint32_t[]> indices32;
	std::shared_ptr<uint16_t[]> indices16;

	if (head.index_count > 50000000) {
		LOG_ERROR("Too many indices for \"%s\"", filename);
		return r;
	}

	if (head.vertex_count > 65535) {
		indices32 = std::shared_ptr<uint32_t[]>(new uint32_t[head.index_count]);
		indexBuffer = indices32.get();
		sizeOfIndices = sizeof(uint32_t);
	}
	else {
		indices16 = std::shared_ptr<uint16_t[]>(new uint16_t[head.index_count]);
		indexBuffer = indices16.get();
		sizeOfIndices = sizeof(uint16_t);
	}

	if (indices32 == nullptr && indices16 == nullptr)
	{
		LOG_ERROR("No indices read");
		return r;
	}

	auto vertices = std::shared_ptr<vbo_layout_vnut[]>(new vbo_layout_vnut[head.vertex_count]);

	read = fread(vertices.get(), sizeof(*vertices.get()), head.vertex_count, f);
	if (read != head.vertex_count) {
		LOG_ERROR("Error, %d vertices expected, read %zd", head.vertex_count, read);
		return r;
	}

	read = fread(indexBuffer, sizeOfIndices, head.index_count, f);
	if (read != head.index_count) {
		LOG_ERROR("Error, %d indices expected, read %zd", head.index_count, read);
		return r;
	}

	r.storeVertices(vertices, head.vertex_count);
	if (indices16)
	{
		r.storeIndices(indices16, head.index_count);
	}
	else
	{
		r.storeIndices(indices32, head.index_count);
	}

	return r;
}

static void updateClbk(HgEntity* e, uint32_t tdelta) {
	//	printf("cube\n");
}

static void destroy(HgEntity* e) {
	e->destroy();
}

int8_t model_data::load(HgEntity* entity, const char* filename) {
	//this needs to be per model instance if the model is animated
	auto rd = RenderData::Create();
	entity->setRenderData(rd);

	entity->setDestroy(false);

	model_data mdl( LoadModel(filename) );

	auto vertices = mdl.getVertices();
	auto indices16 = mdl.getIndices16();
	auto indices32 = mdl.getIndices32();

	if ( vertices == nullptr ||
		(indices16 == nullptr)&&(indices32 == nullptr)) return -1;

	if (indices16 != nullptr) {
		MeshMath::computeTangents(vertices.get(), mdl.getVertexCount(), indices16.get(), mdl.getIndexCount());
	}
	else if (indices32 != nullptr) {
		MeshMath::computeTangents(vertices.get(), mdl.getVertexCount(), indices32.get(), mdl.getIndexCount());
	}

	/*
	for (int i = 0; i < mdl.vertex_count; i++) {
		float x = mdl.vertices[i].tan.x;
		float y = mdl.vertices[i].tan.y;
		float z = mdl.vertices[i].tan.z;
		float w = mdl.vertices[i].tan.w;
		printf("%f %f %f %f\n",x, y, z, w);
	}
*/
	auto record = HgVbo::GenerateFrom(vertices.get(), mdl.getVertexCount());
	rd->VertexVboRecord(record);

	if (indices16 != nullptr) {
		auto iRec = HgVbo::GenerateUniqueFrom(indices16.get(), mdl.getIndexCount());
		rd->indexVboRecord(iRec);
	}
	else if (indices32 != nullptr) {
		auto iRec = HgVbo::GenerateUniqueFrom(indices32.get(), mdl.getIndexCount());
		rd->indexVboRecord(iRec);
	}

	entity->setDestroy(false);

	return 0;
}

bool model_data::load_ini(HgEntity* entity, std::string filename) {
	IniLoader::Contents contents = IniLoader::parse(filename);
	return load_ini(entity, contents);
}

bool model_data::load_ini(HgEntity* entity, const IniLoader::Contents& contents) {
	//this needs to be per model instance if the model is animated
	auto rd = RenderData::Create();
	entity->setRenderData(rd);

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

	if (model_data::load(entity, modelFilename.c_str()) < 0) return false;

	entity->scale(scale);
	entity->origin(origin.scale(1.0f/scale));
	entity->position(position);
	entity->orientation(orientation);

	auto renderData = entity->getRenderDataPtr().get();

	if (!vertexShader.empty() && !fragmentShader.empty())
		renderData->getMaterial().setShader( HgShader::acquire(vertexShader.c_str(), fragmentShader.c_str()) );

	if (!diffuseTexture.empty())
	{
		auto tmp = HgTexture::acquire(diffuseTexture, HgTexture::DIFFUSE);
		if (tmp != nullptr) {

			renderData->getMaterial().addTexture(tmp);
		}
	}

	if (!specularTexture.empty())
	{
		auto tmp = HgTexture::acquire(specularTexture, HgTexture::SPECULAR);
		if (tmp != nullptr) {
			renderData->getMaterial().addTexture(tmp);
		}
	}

	if (!normalTexture.empty())
	{
		auto tmp = HgTexture::acquire(normalTexture, HgTexture::NORMAL);
		if (tmp != nullptr) {
			renderData->getMaterial().addTexture(tmp);
		}
	}

	return true;
}

void HgModel::Load(std::string filename)
{
	if (filename.find_last_of(".ini"))
	{
		m_model.load_ini(&m_entity, filename);
	}
}

void HgModel::Load(const IniLoader::Contents& ini)
{
	m_model.load_ini(&m_entity, ini);
}

void HgModel::getInstanceData(Instancing::GPUTransformationMatrix* instanceData)
{
	const auto mat = getEntity().computeWorldSpaceMatrix();
	mat.store(instanceData->matrix);
}

//REGISTER_LINKTIME(hgmodel,change_to_model);
REGISTER_LINKTIME2(HgModel, HgModel);