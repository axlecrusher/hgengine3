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

std::unique_ptr<FILE, decltype(&fclose)> open(const char* filename) {
	std::unique_ptr<FILE, decltype(&fclose)> ret(nullptr,fclose);

	FILE* f = NULL;
	errno_t err = fopen_s(&f, filename, "rb");
	if (err != 0) {
		fprintf(stderr, "Unable to open file \"%s\"\n", filename);
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
		fprintf(stderr, "Unable to read file header for \"%s\"\n", filename);
		return r;
	}

	//if (head.vertex_count > 65535) {
	//	fprintf(stderr, "Too many vertices for \"%s\"\n", filename);
	//	fclose(f);
	//	return r;
	//}

	int sizeOfIndices = 0;
	void* indexBuffer = nullptr;

	std::shared_ptr<uint32_t[]> indices32;
	std::shared_ptr<uint16_t[]> indices16;

	if (head.index_count > 50000000) {
		fprintf(stderr, "Too many indices for \"%s\"\n", filename);
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
		fprintf(stderr, "No indices read");
		return r;
	}

	auto vertices = std::shared_ptr<vbo_layout_vnut[]>(new vbo_layout_vnut[head.vertex_count]);

	read = fread(vertices.get(), sizeof(*vertices.get()), head.vertex_count, f);
	if (read != head.vertex_count) {
		fprintf(stderr, "Error, %d vertices expected, read %zd", head.vertex_count, read);
		return r;
	}

	read = fread(indexBuffer, sizeOfIndices, head.index_count, f);
	if (read != head.index_count) {
		fprintf(stderr, "Error, %d indices expected, read %zd", head.index_count, read);
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

static void* change_to_model(HgEntity* entity) {
	//this needs to be per model instance if the model is animated
	auto rd = RenderData::Create();
	entity->setRenderData( rd );
	return nullptr;
}

int8_t model_data::load(HgEntity* entity, const char* filename) {
	change_to_model(entity);

	auto rd = entity->renderData();

	entity->setDestroy(false);

	model_data mdl( LoadModel(filename) );

	auto vertices = mdl.getVertices();
	auto indices16 = mdl.getIndices16();
	auto indices32 = mdl.getIndices32();

	if ( vertices == nullptr ||
		(indices16 == nullptr)&&(indices32 == nullptr)) return -1;

	if (indices16 != nullptr) {
		computeTangents(vertices.get(), mdl.getVertexCount(), indices16.get(), mdl.getIndexCount());
	}
	else if (indices32 != nullptr) {
		computeTangents(vertices.get(), mdl.getVertexCount(), indices32.get(), mdl.getIndexCount());
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
	change_to_model(entity);

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

	auto renderData = entity->renderData();

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

template<typename T>
float floatFromNormalInt(T x)
{
	const float max = std::numeric_limits<T>::max();
	return x / max;
}

//computes tangents for a triangle as defined by 3 consecutive indices
void computeTangentsTriangle(const vbo_layout_vnut* vertices, uint32_t* indices, vector3f* tangent, vector3f* bitangent)
{
	const auto& v0 = vertices[indices[0]];
	const auto& v1 = vertices[indices[1]];
	const auto& v2 = vertices[indices[2]];

	//printf("%d %d\n", v0.uv.u, v0.uv.v);

	const auto e1 = v1.v.object - v0.v.object;
	const auto e2 = v2.v.object - v0.v.object;

	const auto x1 = floatFromNormalInt(v1.uv.u) - floatFromNormalInt(v0.uv.u);
	const auto x2 = floatFromNormalInt(v2.uv.u) - floatFromNormalInt(v0.uv.u);
	const auto y1 = floatFromNormalInt(v1.uv.v) - floatFromNormalInt(v0.uv.v);
	const auto y2 = floatFromNormalInt(v2.uv.v) - floatFromNormalInt(v0.uv.v);

	const float r = 1.0f / (x1 * y2 - x2 * y1);
	const vector3f t = (e1.scale(y2) - e2.scale(y1)).scale(r);
	const vector3f b = (e2.scale(x1) - e1.scale(x2)).scale(r);

	tangent[indices[0]] += t;
	tangent[indices[1]] += t;
	tangent[indices[2]] += t;

	bitangent[indices[0]] += b;
	bitangent[indices[1]] += b;
	bitangent[indices[2]] += b;
}



REGISTER_LINKTIME(hgmodel,change_to_model);