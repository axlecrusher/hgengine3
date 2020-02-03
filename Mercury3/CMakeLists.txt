file(GLOB HGSOUNDSRC
	"HgSound/*.cpp"
	"HgSound/*.h"
	lib/libsoundio/soundio/*.*
)

file(GLOB OGGINCLUDE
	lib/x64/libogg/include
	lib/x64/vorbis/include
)

file(GLOB OGGLIBS
	lib/x64/vorbis/lib/*.lib
)

source_group("HgSound" FILES ${HGSOUNDSRC})

file(GLOB RENDERERSRC
	"src/ogl/*.cpp"
	"src/ogl/*.h"
	"src/renderer/*.cpp"
	"src/renderer/*.h"
)

source_group("RenderBackend" FILES ${RENDERERSRC})

file(GLOB CORE_ENGINESOURCES
	"src/core/*.h"
	"src/core/*.cpp"
)

source_group("Core" FILES ${CORE_ENGINESOURCES})

file(GLOB ENGINESOURCES
	"*.cpp"
	"*.h"
	"src/*.cpp"
	"src/*.h"
	"src/math/*.cpp"
	"src/math/*.h"
	"str_utils.c"
	"FileWatch.c"
	"LinkedList.c"
	"symbol_enumerator.c"
	"ini.c"
)

source_group("Source" FILES ${ENGINESOURCES})

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
    set(WIDECHAR "/Zc:wchar_t")
endif()

add_library(hgengine STATIC ${ENGINESOURCES} ${HGSOUNDSRC} ${RENDERERSRC} ${CORE_ENGINESOURCES} ${OPENVR_LIB} ${OGGLIBS})
target_compile_options(hgengine PRIVATE ${WIDECHAR})

target_include_directories(hgengine PUBLIC "./")
target_include_directories(hgengine PUBLIC "include/" "include/stb")
target_include_directories(hgengine PUBLIC ${OGGINCLUDE})
target_include_directories(hgengine PUBLIC "src/")
target_include_directories(hgengine PUBLIC "HgSound/")
target_include_directories(hgengine PUBLIC "src/ogl/")
target_include_directories(hgengine PUBLIC "src/renderer/")
target_include_directories(hgengine PUBLIC "lib/libsoundio/")
target_include_directories(hgengine PUBLIC "src/renderer/")
target_include_directories(hgengine PUBLIC "src/core")
target_include_directories(hgengine PUBLIC "src/datastructures/")
