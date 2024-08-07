cmake_minimum_required(VERSION 3.30)
project("HGENGINE3")

set(GTEST_ROOT "C:/Program Files/googletest-distribution")

enable_testing()
find_package(GTest REQUIRED)

if(MSVC)
	set(DBGHELP_LIBRARY "dbghelp")
endif()

set(HGENGINE_INCLUDES "Mercury3")

find_package(OpenGL)
find_library(GLEW_LIBRARY PATHS Mercury3/lib/x64 NAMES glew32s)
find_path(GLEW_INCLUDE_DIR Mercury3/include)
find_library(SOUNDIO_LIB PATHS Mercury3/lib/libsoundio/x86_64 NAMES soundio)

find_library(OPENVR_LIB PATHS Mercury3/lib/openvr NAMES openvr_api64 PATH_SUFFIXES lib)
add_subdirectory("Mercury3")
add_subdirectory("HgPlayground")

if(GTEST_FOUND)
	add_subdirectory("Tests")
endif()