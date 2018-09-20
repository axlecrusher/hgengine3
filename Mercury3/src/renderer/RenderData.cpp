#include <RenderData.h>
#include <RenderBackend.h>

RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:blendMode(BlendMode::BLEND_NORMAL),
	renderFlags(RenderFlags(FACE_CULLING | DEPTH_WRITE)), instanceCount(0)
	, gpuBuffer(nullptr)
{

}

RenderData::~RenderData()
{
	destroy();
}

void RenderData::render() {
	RENDERER()->setRenderAttributes(blendMode, renderFlags);
	hgVbo()->use();
	if (colorVbo() != nullptr) colorVbo()->use();
	indexVbo()->use();
	indexVbo()->draw(this);
}

void RenderData::destroy() {
	//FIXME: Do something to clean up hgVbo
	//hgvbo_remove(d->hgvbo, d->vbo_offset, d->vertex_count)

	if (shader) HgShader::release(shader);
	shader = nullptr;
}

void RenderData::init() {
	shader = HgShader::acquire("test_vertex.glsl", "test_frag.glsl");
}

void RenderData::updateGpuTextures() {
	clearTextureIDs();

	for (auto itr = textures.begin(); itr != textures.end(); itr++) {
		auto texture = *itr;
		if (texture->NeedsGPUUpdate()) {
			texture->sendToGPU();
			HgTexture::TextureType type = texture->getType();
		}

		//FIXME: Share texture pointers can cause a problem here. Texture may be updated by another HgElement.
		//refactor into HgTexture making callback into renderData to set texture IDs.
		//NOTE: Not sure this applies anymore. HgTexture is pretty immutable after creation.
		setTexture(texture.get());
	}
}