#include <OGLvbo.h>

GLenum hgPrimitiveTypeToGLType(PrimitiveType t)
{
	switch (t)
	{
	case PrimitiveType::TRIANGLES:
		return GL_TRIANGLES;
	case PrimitiveType::LINES:
		return GL_LINES;
	default:
		return GL_TRIANGLES;
	}
}
