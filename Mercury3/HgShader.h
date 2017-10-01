#pragma once

class HgShader {
	public:
		typedef HgShader*(*createShaderCallback)(const char* vert, const char* frag);
		virtual void load() = 0;
		virtual void destroy() = 0;
		virtual void enable() = 0;

		static HgShader* acquire(const char* vert, const char* frag);
		static void release(HgShader* shader);

		static createShaderCallback Create;
};