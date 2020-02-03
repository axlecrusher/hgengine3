find_path(GLEW_INCLUDE_DIR GL/glew.h)
find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)

file(GLOB PLAYGROUNDSRC
	"HgPlayground.cpp"
)

if(MSVC)
    add_definitions(-DUNICODE -D_UNICODE)
    set(WIDECHAR "/Zc:wchar_t")
endif()

add_executable(hgplayground ${PLAYGROUNDSRC})
target_compile_options(hgplayground PRIVATE ${WIDECHAR})

set(HGENGINE_INCLUDES "../Mercury3")

file(GLOB OGGLIBS
	../Mercury3/lib/x64/libogg/lib/*.lib
	../Mercury3/lib/x64/vorbis/lib/*.lib
)

SET_TARGET_PROPERTIES(hgplayground PROPERTIES LINK_FLAGS "/WHOLEARCHIVE:hgengine.lib")
#SET_TARGET_PROPERTIES(hgplayground PROPERTIES LINK_FLAGS_RELEASE "/WHOLEARCHIVE:hgengine.lib")
#SET_TARGET_PROPERTIES(hgplayground PROPERTIES LINK_FLAGS_DEBUG "/WHOLEARCHIVE:hgengine.lib")

target_link_libraries (hgplayground LINK_PUBLIC hgengine ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY} ${DBGHELP_LIBRARY} ${SOUNDIO_LIB} xaudio2.lib ${OPENVR_LIB} Winmm.lib ${OGGLIBS})

include_directories(${GLEW_INCLUDE_DIRS})
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/include/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/HgSound/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/ogl/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/renderer/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/lib/libsoundio/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/renderer/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/lib/openvr/include/")
