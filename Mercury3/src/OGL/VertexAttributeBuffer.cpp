#include <VertexAttributeBuffer.h>
#include <Logging.h>

void GLVertexAttributeBuffer::AllocateOnGPU(size_t sizeBytes)
{
	if (bufferId.value == 0)
	{
		Init();
	}

	if (m_maxSize < sizeBytes) {
		//grow buffer size
		glBindBuffer(GL_ARRAY_BUFFER, bufferId.value);
		glBufferData(GL_ARRAY_BUFFER, sizeBytes, nullptr, GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_maxSize = sizeBytes;
	}
}

void GLVertexAttributeBuffer::toGPU(const void* data, const size_t size)
{
	AllocateOnGPU(size);

	//Using mapped buffers seems to be faster than glBufferData or glBufferSubData
	auto mapped = getGPUMemoryPtr();
	memcpy(mapped.ptr, data, size);

	//if (m_maxSize < size) {
	//	//grow buffer and load all data
	//	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
	//}
	//else {
	//	//load all data
	//	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	//}

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MatrixVertexAttribute::SendToGPU(const IHgGPUBuffer* bufferObject)
{
	m_attributeBuffer.toGPU(bufferObject->getBufferPtr(), bufferObject->sizeBytes());
}

void InvalidAttributeError(const HgShader& shader, const std::string& name)
{
	auto ss = shader.sourceStruct();
	LOG_ERROR("Invalid Attribute: %s (shader %s)", name.c_str(), ss->vert_file_path.c_str());
}

void MatrixVertexAttribute::Setup(const Instancing::InstancingMetaData& imd, const HgShader& shader)
{
	const auto attribLocation = shader.getAttributeLocation(m_attributeName);
	if (attribLocation < 0)
	{
		InvalidAttributeError(shader, m_attributeName);
		return;
	}

//	LOG("%s : %d", m_attributeName.c_str(), m_attributeBuffer.getValue());

	//glVertexAttribPointer requires a buffer be bound
	glBindBuffer(GL_ARRAY_BUFFER, m_attributeBuffer.getValue());

	constexpr size_t stride = sizeof(float) * 16;

	for (int i = 0; i < 4; i++)
	{
		const size_t byteOffset = imd.byteOffset + (sizeof(float) * 4 * i);
		const auto location = attribLocation + i;
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, (void*)byteOffset);
		glVertexAttribDivisor(location, 1); //advance per instance drawn
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	setNeedSetup(false);
}

MappedMemory MatrixVertexAttribute::getGPUMemoryPtr()
{
	MappedMemory mb(this);

	glBindBuffer(GL_ARRAY_BUFFER, m_attributeBuffer.getValue());

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

void MatrixVertexAttribute::ReleaseMappedMemory(MappedMemory* mm)
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}