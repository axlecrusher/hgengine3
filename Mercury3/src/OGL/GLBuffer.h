#pragma once

#include <OGLheaders.h>
#include <RefCounterTable.h>

struct GLVaoId
{
	GLVaoId() : value(0)
	{}

	~GLVaoId() {
		Destroy();
	}

	GLVaoId(const GLVaoId& rhs)
	{
		value = 0; //does not copy
	}

	GLVaoId(GLVaoId&& rhs)
	{
		Destroy();
		value = rhs.value;
		rhs.value = 0;
	}

	const GLVaoId& operator=(const GLVaoId& rhs) = delete;
	const GLVaoId& operator=(GLVaoId&& rhs)
	{
		Destroy();
		value = rhs.value;
		rhs.value = 0;
		return *this;
	}

	//inline operator GLuint() const { return value; }

	//Instantiate a new VAO in OPENGL. Will destroy and rebuild an existing VAO.
	bool Init();

	//Deallocate in opengl
	void Destroy();

	//Use this VAO. Will initialize if it has not yet been initialized.
	void Enable();
	static void Disable();

	GLuint value;
};

using VaoManager = RefCountedTable< GLVaoId >;
using VaoIndex = VaoManager::IndexType;