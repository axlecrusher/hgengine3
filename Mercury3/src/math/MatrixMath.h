#pragma once

#include <HgMath.h>
#include <HgTypes.h>
#include <vectorial/vectorial.h>

struct SPI; //forward declare

namespace HgMath {
	inline vector3f operator*(const mat4f& m, vector3f p) {
		using namespace vectorial;
		const auto t = vectorial::vec3f(p.raw());
		const auto r = m * t.xyz1();
		float tmp[4];
		r.store(tmp);
		return vector3f(tmp[0], tmp[1], tmp[2]);
	}

	inline mat4f toTranslationMatrix(const vector3f& p)
	{
		const vectorial::vec3f v(p.raw());
		return mat4f::translation(v);
	}

	//Compute local transformation matrix
	HgMath::mat4f computeTransformMatrix(const SPI& sd, const bool applyScale = true, bool applyRotation = true, bool applyTranslation = true);
}