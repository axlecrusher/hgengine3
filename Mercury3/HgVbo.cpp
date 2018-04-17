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


namespace HgVbo {
	template<>
	IHgVbo* Create<vbo_layout_vc>() {
		return vbo_from_api_type<vbo_layout_vc>();
	}

	template<>
	IHgVbo* Create<vbo_layout_vn>() {
		return vbo_from_api_type<vbo_layout_vn>();
	}

	template<>
	IHgVbo* Create<vbo_layout_vnu>() {
		return vbo_from_api_type<vbo_layout_vnu>();
	}

	template<>
	IHgVbo* Create<vbo_layout_vnut>() {
		return vbo_from_api_type<vbo_layout_vnut>();
	}

	template<>
	IHgVbo* Create<uint8_t>() {
		return vbo_from_api_type<uint8_t>();
	}

	template<>
	IHgVbo* Create<uint16_t>() {
		return vbo_from_api_type<uint16_t>();
	}

	template<>
	IHgVbo* Create<color>() {
		return vbo_from_api_type<color>();
	}
}

IHgVbo* staticVbo = HgVbo::Create<vbo_layout_vc>();
IHgVbo* staticVboVNU = HgVbo::Create<vbo_layout_vnu>();
IHgVbo* staticVboVNUT = HgVbo::Create<vbo_layout_vnut>();
IHgVbo* staticIndice8 = HgVbo::Create<uint8_t>();
IHgVbo* staticIndice16 = HgVbo::Create<uint16_t>();