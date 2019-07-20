#include <RenderBackend.h>
#include <OGLBackend.h>

#include <oglDisplay.h>
#include <algorithm>

namespace Renderer {
	void Init() {
		RENDERER()->Init();
	}
}

RenderBackend* RENDERER() {
	//replace with some kind of configure based factory thing
	static RenderBackend* api = OGLBackend::Create();
	return api;
}

static void submit_for_render_serial(const Viewport& vp, RenderData* renderData, const float* worldSpaceMatrix, const HgMath::mat4f& viewMatrix, const HgMath::mat4f& projection) {
	RENDERER()->setViewport(vp);

	//load texture data to GPU here. Can this be made to be done right after loading the image data, regardless of thread?
	renderData->getMaterial().updateGpuTextures();

	HgShader& shader = renderData->getMaterial().getShader();
	//if (shader) {
		shader.enable();
		//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
		//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated
		//shader->setGlobalUniforms(*camera);
		//const auto spacial = e->getSpacialData();
		shader.uploadMatrices(worldSpaceMatrix, projection, viewMatrix);
		shader.setLocalUniforms(*renderData);
	//}

	renderData->render();
}

void Renderer::Render(const Viewport& vp, const HgMath::mat4f& viewMatrix, const HgMath::mat4f& projection, RenderQueue* queue)
{
	for (auto& renderInstance : queue->getOpaqueQueue()) {
		submit_for_render_serial(vp, renderInstance.renderData.get(), renderInstance.worldSpaceMatrix, viewMatrix, projection);
	}

	for (auto& renderInstance : queue->getTransparentQueue()) {
		submit_for_render_serial(vp, renderInstance.renderData.get(), renderInstance.worldSpaceMatrix, viewMatrix, projection);
	}
}

//void Renderer::Render(uint8_t viewportIdx, HgCamera* camera, const HgMath::mat4f& projection, RenderQueue* queue)
//{
//	const auto viewMatrix = camera->toViewMatrix();
//	Render(viewportIdx, viewMatrix, projection, queue);
//}

void RenderQueue::Enqueue(RenderDataPtr& renderData)
{
	if (renderData)
	{
		const auto worldSpaceMatrix = HgMath::mat4f::identity();
		Enqueue(renderData, worldSpaceMatrix, 0);
	}
}


void RenderQueue::Enqueue(const HgEntity* e)
{
	auto renderData = e->getRenderDataPtr();
	if (renderData)
	{
		const auto worldSpaceMatrix = e->computeWorldSpaceMatrix();
		Enqueue(renderData, worldSpaceMatrix, e->getDrawOrder());
	}
}

void RenderQueue::Enqueue(RenderDataPtr& rd, const HgMath::mat4f& wsm, int8_t drawOrder)
{
	if (rd->getMaterial().isTransparent()) {
		//order by distance back to front?
		m_transparentEntities.emplace_back(wsm, rd, drawOrder);
	}
	else {
		//order by distance front to back?
		m_opaqueEntities.emplace_back(wsm, rd, drawOrder);
	}
}



void RenderQueue::Finalize()
{
	sort(m_opaqueEntities);
	sort(m_transparentEntities);
}

void RenderQueue::sort(std::vector<RenderInstance>& v)
{
	std::sort(v.begin(), v.end(),
		[](RenderInstance& a, RenderInstance& b)
	{
		return b.drawOrder > a.drawOrder;
	});
}
