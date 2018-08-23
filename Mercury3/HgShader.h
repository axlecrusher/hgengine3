#pragma once

#include "quaternion.h"
#include <HgTypes.h>
#include <HgCamera.h>
#include <memory>

#include <RenderData.h>
#include <math/matrix.h>

class RenderData;

class HgShader {
	public:
		virtual ~HgShader() {};

		virtual void load() = 0;
		virtual void destroy() = 0;
		virtual void enable() = 0;

		virtual void setGlobalUniforms(const HgCamera& c) = 0;
		virtual void setLocalUniforms(const quaternion* rotation, const point* position, float scale, const point* origin, const RenderData*, const HgCamera* camera) = 0;
		virtual void uploadMatrices(const HgMath::mat4f& movelView, const HgMath::mat4f& projection) = 0;

		static HgShader* acquire(const char* vert, const char* frag);
		static void release(HgShader* shader);

		typedef std::unique_ptr<HgShader>(*createShaderCallback)(const char* vert, const char* frag);
		static createShaderCallback Create;
};