
set(HGENGINE_INCLUDES "../Mercury3")

file(GLOB TEST_SRC
	"*.cpp"
	"*.h"
	"${HGENGINE_INCLUDES}/HgMath.cpp"
	"${HGENGINE_INCLUDES}/HgMath.h"
	"${HGENGINE_INCLUDES}/quaternion.cpp"
	"${HGENGINE_INCLUDES}/quaternion.h"
	"${HGENGINE_INCLUDES}/quaternion_dual.cpp"
	"${HGENGINE_INCLUDES}/quaternion_dual.h"
	"${HGENGINE_INCLUDES}/src/math/vector.h"
	"${HGENGINE_INCLUDES}/src/math/matrix.h"
	"${HGENGINE_INCLUDES}/vertex.h"
)

source_group("Source" FILES ${TEST_SRC})

if(MSVC)
#    add_definitions(-DUNICODE -D_UNICODE)
#    set(WIDECHAR "/Zc:wchar_t")CMAKE_CXX_FLAGS_RELWITHDEBINFO 
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
endif()

add_executable(hgtests ${TEST_SRC})
target_compile_options(hgtests PRIVATE ${WIDECHAR})

target_link_libraries (hgtests LINK_PUBLIC hgengine ${GTEST_LIBRARIES})

include_directories(${GLEW_INCLUDE_DIRS})
target_include_directories(hgengine PUBLIC "${GTEST_INCLUDE_DIRS}/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/include/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/HgSound/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/ogl/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/renderer/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/lib/libsoundio/")
target_include_directories(hgengine PUBLIC "${HGENGINE_INCLUDES}/src/renderer/")
