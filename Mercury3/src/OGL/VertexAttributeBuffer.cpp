#include <VertexAttributeBuffer.h>

void GLVertexAttributeBuffer::toGPU(const void* data, const size_t size)
{
	if (bufferId.value == 0)
	{
		Init();
	}

	glBindBuffer(GL_ARRAY_BUFFER, bufferId.value);

	if (m_maxSize < size) {
		//grow buffer and load all data
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}
	else {
		//load all data
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	m_maxSize = size;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MatrixVertexAttribute::SendToGPU(const IHgGPUBuffer* bufferObject)
{
	m_attributeBuffer.toGPU(bufferObject->getBufferPtr(), bufferObject->sizeBytes());
}

void MatrixVertexAttribute::Setup(const Instancing::InstancingMetaData& imd, const HgShader& shader)
{
	//get the attribute location from the shader
	OGLShaderHandle program_id = shader.getProgramHandle();

	const auto attribLocation = glGetAttribLocation(program_id, m_attributeName.c_str());

	if (attribLocation < 0)
	{
		auto ss = shader.sourceStruct();
		fprintf(stderr, "Invalid Attribute: %s (shader %s)\n", m_attributeName.c_str(), ss->vert_file_path.c_str());
		return;
	}

	//glVertexAttribPointer requires a buffer be bound
	glBindBuffer(GL_ARRAY_BUFFER, m_attributeBuffer.getValue());

	constexpr size_t stride = sizeof(float) * 16;

	for (int i = 0; i < 4; i++)
	{
		const size_t byteOffset = imd.byteOffset + (sizeof(float) * 4 * i);
		glEnableVertexAttribArray(attribLocation + i);
		glVertexAttribPointer(attribLocation + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)byteOffset);
		glVertexAttribDivisor(attribLocation + i, 1); //advance per instance drawn
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	setNeedSetup(false);
}