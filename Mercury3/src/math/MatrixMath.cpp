#include <math/MatrixMath.h>
#include <HgEntity.h>
#include <spi.h>

namespace HgMath {

	HgMath::mat4f computeTransformMatrix(const SPI& sd, const bool applyScale, bool applyRotation, bool applyTranslation)
	{
		//translate to origin, scale, rotate, apply local translation, apply parent transforms
		HgMath::mat4f modelMatrix;
		const auto origin_vec = -vectorial::vec3f(sd.origin.raw());
		//modelMatrix = HgMath::mat4f::translation(origin_vec);

		const float scaleFactor = (sd.scale*applyScale) + (!applyScale * 1.0f); //Integral promotion of bool, avoid branching

		//const auto correct = HgMath::mat4f::scale(scaleFactor) * modelMatrix;
		//I think this is the same result with less math
		modelMatrix = HgMath::mat4f::scale(scaleFactor);
		modelMatrix.value.w = (origin_vec * scaleFactor).xyz1().value;

		if (applyRotation) {
			HgMath::mat4f rotation;
			sd.orientation.toMatrix4(rotation);
			//const auto rotation = sd.orientation.toMatrix4();
			modelMatrix = rotation * modelMatrix;
		}

		const float translationScalar = applyTranslation * 1.0f; //Integral promotion of bool, avoid branching
		//if (applyTranslation) {
			//auto correct = HgMath::mat4f::translation(vectorial::vec3f(position().raw())) * modelMatrix;
			//I think this is the same result with less math
		const auto tmp = vectorial::vec4f(modelMatrix.value.w)
			+ vectorial::vec3f(sd.position.raw()).xyz0() * translationScalar;
		modelMatrix.value.w = tmp.value;
		//}

		return modelMatrix;
	}

}