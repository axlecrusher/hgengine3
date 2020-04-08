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

static void submit_for_render_serial(const Viewport& vp, const RenderInstance& ri, const HgMath::mat4f& viewMatrix, const HgMath::mat4f& projection) {
	RenderData* renderData = ri.renderData.get();
	const float* worldSpaceMatrix = ri.interpolatedWorldSpaceMatrix;

	RENDERER()->setViewport(vp);

	//load texture data to GPU here. Can this be made to be done right after loading the image data, regardless of thread?
	renderData->getMaterial().updateGpuTextures();

	HgShader& shader = renderData->getMaterial().getShader();
	bool shaderCompiled = shader.compile();

	if (shaderCompiled)
	{
		//if the shader recompiled, throw out the old VAO and build a new one
		renderData->vao.Init();
	}
	renderData->vao.Enable();

	auto& imd = ri.imd;
	if (imd.isValid())
	{
		if (imd.instanceData->NeedsLoadToGPU())
		{
			imd.instanceData->sendToGPU();
		}

		//if (shaderCompiled)
		//{
		//	//if the shader recompiled, we need to get the new attribute locations
		//	imd.instanceData->getUseClass()->setNeedSetup(true);
		//}

		//if (imd.instanceData->getUseClass()->getNeedsSetup())
		//{
			imd.instanceData->getUseClass()->Setup(imd, shader);
		//}
	}

	shader.enable();


	//if (shader) {
		//shader.enable();
		//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
		//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated
		//shader->setGlobalUniforms(*camera);
		//const auto spacial = e->getSpacialData();
		shader.uploadMatrices(worldSpaceMatrix, projection, viewMatrix);

		ShaderUniforms uniforms;
		uniforms.material = &(renderData->getMaterial());
		//uniforms.gpuBuffer = renderData->gpuBuffer.get();
		uniforms.remainingTime = &ri.remainingTime;

		shader.setLocalUniforms(uniforms);
	//}

		if (ri.imd.isValid())
		{
			renderData->render(&ri.imd);
		}
		else
		{
			renderData->render();

		}

		renderData->vao.Disable();
}

HgMath::mat4f ViewportRenderTarget::getPerspectiveMatrix() const
{
	HgMath::mat4f projectionMatrix;

	float projection[16];

	const double width = getWidth();
	const double height = getHeight();
	const double aspect = width / height;

	Perspective2(60, aspect, 0.1f, 100.0f, projection);
	projectionMatrix.load(projection);

	return projectionMatrix;
}

HgMath::mat4f ViewportRenderTarget::getOrthoMatrix() const
{
	return vectorial::transpose(HgMath::mat4f::ortho(-1, 1, -1, 1, -1, 1));
}

void Renderer::Render(const Viewport& vp, const HgMath::mat4f& viewMatrix, const HgMath::mat4f& projection, RenderQueue* queue)
{
	for (auto& renderInstance : queue->getOpaqueQueue()) {
		submit_for_render_serial(vp, renderInstance, viewMatrix, projection);
	}

	for (auto& renderInstance : queue->getTransparentQueue()) {
		submit_for_render_serial(vp, renderInstance, viewMatrix, projection);
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

		vector3f velocity;
		Enqueue(renderData, worldSpaceMatrix, 0, velocity);
	}
}

void RenderQueue::Enqueue(const Instancing::InstancingMetaData& imd)
{
	if (imd.renderData)
	{
		RenderDataPtr rd = imd.renderData;
		if (imd.instanceData->NeedsLoadToGPU())
		{
			imd.instanceData->sendToGPU();
		}

		const auto worldSpaceMatrix = HgMath::mat4f::identity();

		vector3f velocity;

		//RenderInstance i()
		Enqueue(rd, worldSpaceMatrix, 0, velocity, &imd);
	}
}

velocity ComputeVelocity(const HgMath::mat4f& worldSpace, HgEntity* e, const HgTime& dt)
{
	//try to compute the velocity of an entity based on the previous rendered position
	point currentPosition;

	auto& spd = e->getSpacialData();
	bool hasPrevious = spd.hasPrevious();

	//compute the position based on this game update loop result
	transformPoint(worldSpace, vectorial::vec3f::zero()).store(currentPosition.raw());
	const auto delta = currentPosition - spd.PreviousPosition();

	//store the current position for use in the next render
	spd.PreviousPosition(currentPosition);

	if (hasPrevious)
		return (delta / dt); //velocity

	return velocity();
}

void RenderQueue::Enqueue(HgEntity* e, HgTime dt)
{
	auto renderData = e->getRenderDataPtr();
	if (renderData)
	{
		const auto worldSpaceMatrix = e->computeWorldSpaceMatrix();
		const auto vel = ComputeVelocity(worldSpaceMatrix, e, dt);
		Enqueue(renderData, worldSpaceMatrix, e->getDrawOrder(), vel);
	}
}

void RenderQueue::Enqueue(RenderDataPtr& rd, const HgMath::mat4f& wsm, int8_t drawOrder, const vector3f& velocityVector, const Instancing::InstancingMetaData* imd)
{
	if (rd->getMaterial().isTransparent()) {
		//order by distance back to front?
		m_transparentEntities.emplace_back(wsm, rd, velocityVector, drawOrder, imd);
	}
	else {
		//order by distance front to back?
		m_opaqueEntities.emplace_back(wsm, rd, velocityVector, drawOrder, imd);
	}
}

void interpolatePosition(RenderInstance& ri, const HgTime& remain)
{
	const auto deltaPos = ri.velocityVector * remain;
	const auto translate = vectorial::mat4f::translation(HgMath::vec3f(deltaPos));
	const auto worldMatrix = vectorial::mat4f(ri.worldSpaceMatrix);
	const auto matrix = translate * worldMatrix;

	matrix.store(ri.interpolatedWorldSpaceMatrix);
}

void RenderQueue::Finalize(const HgTime& remain)
{
	for (auto& renderInstance : m_opaqueEntities) {
		renderInstance.remainingTime = remain;
		interpolatePosition(renderInstance, remain);
	}

	for (auto& renderInstance : m_transparentEntities) {
		renderInstance.remainingTime = remain;
		interpolatePosition(renderInstance, remain);
	}

	sort(m_opaqueEntities);
	sort(m_transparentEntities);
}

void RenderQueue::sort(std::vector<RenderInstance>& v)
{
	std::sort(v.begin(), v.end(),
		[](RenderInstance& a, RenderInstance& b)
	{
		return b.drawOrder < a.drawOrder;
	});
}
