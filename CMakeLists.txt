project("HGENGINE3")

set(DBGHELP_LIBRARY "dbghelp")

find_package(OpenGL)
find_library(GLEW_LIBRARY PATHS Mercury3/lib/x64 NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
find_path(GLEW_INCLUDE_DIR Mercury3/include)
find_library(SOUNDIO_LIB PATHS Mercury3/lib/libsoundio/x86_64 NAMES soundio)

add_subdirectory("Mercury3")
add_subdirectory("HgPlayground")