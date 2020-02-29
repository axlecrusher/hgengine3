#pragma once

#include <math/matrix.h>
#include <vector>

class HgCamera;
class RenderQueue;

//Group a projection and reder queue to render
struct RenderParams
{
	RenderParams(const HgCamera* c, const HgMath::mat4f* p, RenderQueue* q)
		:camera(c), projection(p), queue(q)
	{}

	const HgCamera* camera;
	const HgMath::mat4f* projection;
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
	virtual HgMath::mat4f getProjectionMatrix() = 0;
	virtual HgMath::mat4f getOrthoMatrix() = 0;

	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;

	//Finish the frame so it may be displayed
	//virtual void Finish() = 0;
};