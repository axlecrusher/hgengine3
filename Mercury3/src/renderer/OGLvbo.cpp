#include <OGLvbo.h>

GLenum hgPrimitiveTypeToGLType(HgEngine::PrimitiveType t)
{
	using namespace HgEngine;

	switch (t)
	{
	case PrimitiveType::TRIANGLES:
		return GL_TRIANGLES;
	case PrimitiveType::LINES:
		return GL_LINES;
	case PrimitiveType::POINTS:
		return GL_POINTS;
	default:
		return GL_TRIANGLES;
	}
}
