#include <glew.h>
#include <VertexAttributeBuffer.h>
#include <Logging.h>

#include <OGLvbo.h>

//TODO: Abstract away the OGL calls

namespace VertexAttributeTypes
{
	void MapToAttributeLocation(const SetupParams& p, int vCount)
	{
		const auto attribLocation = p.shader->getAttributeLocation(*p.attributeName);
		if (attribLocation < 0)
		{
			InvalidAttributeError(p.shader, p.attributeName);
			return;
		}

		//	LOG("%s : %d", m_attributeName.c_str(), m_attributeBuffer.getValue());

			//glVertexAttribPointer requires a buffer be bound
		glBindBuffer(GL_ARRAY_BUFFER, p.attribBuffer->getValue());

		const GLsizei stride = sizeof(float) * 4 * vCount;

		for (int i = 0; i < vCount; i++)
		{
			const size_t byteOffset = p.imd->byteOffset + (sizeof(float) * 4 * i);
			const auto location = attribLocation + i;
			glEnableVertexAttribArray(location);
			glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (void*)byteOffset);
			glVertexAttribDivisor(location, 1); //advance per instance drawn
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//p.iglbuffer->setNeedSetup(false);
	}
}

namespace GraphicsDriverFunctions
{
	void ReleaseMappedMemory(MappedMemory* mm)
	{
		//glBindBuffer(GL_ARRAY_BUFFER, mm.);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	MappedMemory getGPUMemoryPtr(IGPUBuffer* gpuBuffer, GLVertexAttributeBuffer* vab)
	{
		MappedMemory mb(gpuBuffer);

		glBindBuffer(GL_ARRAY_BUFFER, vab->getValue());

		//Using mapped buffers seems to be faster than glBufferData or glBufferSubData
		auto ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (ptr)
		{
			mb.ptr = ptr;
		}
		else
		{
			//error
			const auto error = glGetError();
			LOG_ERROR("OpenGL Error: %d", error);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		return mb;
	}


}

void GLVertexAttributeBuffer::AllocateOnGPU(size_t sizeBytes, BUFFER_USE_TYPE useType)
{
	if (bufferId.value == 0)
	{
		Init();
	}


	if (m_maxSize < sizeBytes) {
		//grow buffer size
		glBindBuffer(GL_ARRAY_BUFFER, bufferId.value);
		glBufferData(GL_ARRAY_BUFFER, sizeBytes, nullptr, VboUseage(useType));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_maxSize = sizeBytes;
	}
}

//void GLVertexAttributeBuffer::toGPU(const void* data, const size_t size)
//{
//	AllocateOnGPU(size);
//
//	//Using mapped buffers seems to be faster than glBufferData or glBufferSubData
//	auto mapped = getGPUMemoryPtr();
//	memcpy(mapped.ptr, data, size);
//}

void InvalidAttributeError(const HgShader* shader, const std::string* name)
{
	auto ss = shader->sourceStruct();
	LOG_ERROR("Invalid Attribute: %s (shader %s)", name->c_str(), ss->vert_file_path.c_str());
}