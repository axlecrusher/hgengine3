#pragma once

class HgShader {
	public:
		virtual void load() = 0;
		virtual void destroy() = 0;
		virtual void enable() = 0;

		static HgShader* acquire(const char* vert, const char* frag);
		static void release(HgShader* shader);
};

//HgShader* HGShader_acquire(const char* vert, const char* frag);
//void HGShader_release(HgShader* shader);

extern HgShader*(*_create_shader)(const char* vert, const char* frag);