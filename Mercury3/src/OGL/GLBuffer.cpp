#include <GLBuffer.h>
#include <memory>

bool GLBufferId::Init()
{
	if (value == 0) {
		glGenBuffers(1, &value);
	}
	return true;
}

void GLBufferId::Destroy()
{
	if (value > 0) {
		glDeleteBuffers(1, &value);
	}
	value = 0;
}

bool GLVaoId::Init()
{
	Destroy();
	glGenVertexArrays(1, &value);
	return true;
}

void GLVaoId::Destroy()
{
	if (value > 0) {
		glDeleteVertexArrays(1, &value);
	}
	value = 0;
}

void GLVaoId::Enable()
{
	if (value == 0) Init();

	glBindVertexArray(value);
}

void GLVaoId::Disable()
{
	glBindVertexArray(0);
}
