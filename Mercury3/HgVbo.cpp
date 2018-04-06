#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>
#include <RenderBackend.h>

#include <OGLvbo.h>
//static void* _currentVbo;

template<typename T>
static IHgVbo* vbo_from_api_type() {
	switch (RENDERER->Type()) {
	case OPENGL:
		return new OGLvbo<T>();
		break;
	default:
		return nullptr;
	}
}

//namespace HgVboFactory {
	template<>
	IHgVbo* new_vbo<vbo_layout_vc>() {
		return vbo_from_api_type<vbo_layout_vc>();
	}

	template<>
	IHgVbo* new_vbo<vbo_layout_vn>() {
		return vbo_from_api_type<vbo_layout_vn>();
	}

	template<>
	IHgVbo* new_vbo<vbo_layout_vnu>() {
		return vbo_from_api_type<vbo_layout_vnu>();
	}

	template<>
	IHgVbo* new_vbo<vbo_layout_vnut>() {
		return vbo_from_api_type<vbo_layout_vnut>();
	}

	template<>
	IHgVbo* new_vbo<uint8_t>() {
		return vbo_from_api_type<uint8_t>();
	}

	template<>
	IHgVbo* new_vbo<uint16_t>() {
		return vbo_from_api_type<uint16_t>();
	}

	template<>
	IHgVbo* new_vbo<color>() {
		return vbo_from_api_type<color>();
	}
//}

IHgVbo* staticVbo = new_vbo<vbo_layout_vc>();
IHgVbo* staticVboVNU = new_vbo<vbo_layout_vnu>();
IHgVbo* staticVboVNUT = new_vbo<vbo_layout_vnut>();
IHgVbo* staticIndice8 = new_vbo<uint8_t>();
IHgVbo* staticIndice16 = new_vbo<uint16_t>();