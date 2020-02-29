#pragma once

#include <math/matrix.h>
#include <vector>

class HgCamera;
class RenderQueue;

class IProjection; //forward declare

//Group a projection and reder queue to render
struct RenderParams
{
	RenderParams(const HgCamera* c, const IProjection* p, RenderQueue* q)
		:camera(c), projection(p), queue(q)
	{}

	const HgCamera* camera;
	const IProjection* projection;
	RenderQueue* queue;
}; 

using RenderParamsList = std::vector< RenderParams >;

class IRenderTarget
{
public:
	~IRenderTarget()
	{}

	virtual bool Init() = 0;
	virtual void Render(const RenderParamsList& l) = 0;
	virtual HgMath::mat4f getPerspectiveMatrix() const = 0;
	virtual HgMath::mat4f getOrthoMatrix() const = 0;

	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;

	//Finish the frame so it may be displayed
	//virtual void Finish() = 0;
};

class IProjection
{
public:
	virtual HgMath::mat4f getProjectionMatrix(const IRenderTarget&) const = 0;
};

class PerspectiveProjection : public IProjection
{
public:
	virtual HgMath::mat4f getProjectionMatrix(const IRenderTarget& t) const
	{
		return t.getPerspectiveMatrix();
	}
};

class OrthographicProjection : public IProjection
{
public:
	virtual HgMath::mat4f getProjectionMatrix(const IRenderTarget& t) const
	{
		return t.getOrthoMatrix();
	}
};