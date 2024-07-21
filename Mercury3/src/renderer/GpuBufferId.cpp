#include <OGLheaders.h>
#include <GpuBufferId.h>

//TODO: Abstract away the OGL calls

bool GpuBufferId::Init()
{
	if (value == 0) {
		glGenBuffers(1, &value);
	}
	return true;
}

void GpuBufferId::Destroy()
{
	if (value > 0) {
		glDeleteBuffers(1, &value);
	}
	value = 0;
}
