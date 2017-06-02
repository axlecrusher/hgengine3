#pragma once

typedef struct HgShader_vtable{
	void(*load)(struct HgShader* shader);
	void(*destroy)(struct HgShader* shader);
	void(*enable)(struct HgShader* shader);
} HgShader_vtable;

typedef struct HgShader {
	HgShader_vtable* vptr;
} HgShader;