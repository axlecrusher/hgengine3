#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>
#include <RenderBackend.h>

#include <OGLvbo.h>
//static void* _currentVbo;

template<typename T>
static std::unique_ptr<IHgVbo> vbo_from_api_type() {
	switch (RENDERER()->Type()) {
	case OPENGL:
		return std::move( std::make_unique< OGLvbo<T> >() );
		break;
	default:
		return nullptr;
	}
}


namespace HgVbo {
	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vc>() {
		return std::move(vbo_from_api_type<vbo_layout_vc>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vn>() {
		return std::move(vbo_from_api_type<vbo_layout_vn>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vnu>() {
		return std::move(vbo_from_api_type<vbo_layout_vnu>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vnut>() {
		return std::move(vbo_from_api_type<vbo_layout_vnut>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<uint8_t>() {
		return std::move(vbo_from_api_type<uint8_t>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<uint16_t>() {
		return std::move(vbo_from_api_type<uint16_t>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<color>() {
		return std::move(vbo_from_api_type<color>());
	}
}

std::shared_ptr<IHgVbo> staticVbo = HgVbo::Create<vbo_layout_vc>();
std::shared_ptr<IHgVbo> staticVboVNU = HgVbo::Create<vbo_layout_vnu>();
std::shared_ptr<IHgVbo> staticVboVNUT = HgVbo::Create<vbo_layout_vnut>();
std::shared_ptr<IHgVbo> staticIndice8 = HgVbo::Create<uint8_t>();
std::shared_ptr<IHgVbo> staticIndice16 = HgVbo::Create<uint16_t>();