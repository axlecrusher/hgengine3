#pragma once

#include <glew.h>

struct GLBufferId
{
	GLBufferId() : value(0)
	{}

	~GLBufferId() {
		Destroy();
	}

	GLBufferId(const GLBufferId& rhs) = delete;
	GLBufferId(GLBufferId&& rhs)
	{
		Destroy();
		value = rhs.value;
		rhs.value = 0;
	}

	const GLBufferId& operator=(const GLBufferId& rhs) = delete;
	const GLBufferId& operator=(GLBufferId&& rhs)
	{
		Destroy();
		value = rhs.value;
		rhs.value = 0;
		return *this;
	}

	inline operator GLuint() const { return value; }

	//Instantiate buffer in OPENGL
	bool Init();

	//Deallocate in opengl
	void Destroy();

	GLuint value;
};

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