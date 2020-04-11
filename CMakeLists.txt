cmake_minimum_required(VERSION 3.10)

project(ga)

set(ENV{CUDACXX} "/usr/local/cuda-9.0/bin/nvcc")
enable_language(CUDA)
include(CheckLanguage)
check_language(CUDA)

find_package(CUDA 9.0 REQUIRED)
message(STATUS "Found CUDA ${CUDA_VERSION_STRING} at ${CUDA_TOOLKIT_ROOT_DIR}")


find_package(OpenGL REQUIRED COMPONENTS OpenGL)
message(STATUS "Found OpenGL ${OpenGL_VERSION_STRING}")
find_package(GLUT REQUIRED)
message(STATUS "Found GLUT ${GLUT_LIBRARIES}")

set(CUDA_LINK_LIBRARIES_KEYWORD PUBLIC)

#cuda_select_nvcc_arch_flags(ARCH_FLAGS) #opcional

if(NOT DEFINED CMAKE_CUDA_STANDARD)
	set(CMAKE_CUDA_STANDARD 14)
	set(CMAKE_CUDA_STANDARD_REQUIRED_ON)
endif()

add_executable (ga
	main.cpp
)

add_dependencies(ga OpenGL::OpenGL)
include_directories(${GLUT_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS} ~/CUDA_SAMPLES/common/inc ~/proyectos/utils)

#target_link_libraries(ncuerpo OpenGL::OpenGL ${GLUT_LIBRARY} GLU)
target_link_libraries(ga ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${CUDA_LIBRARIES} OpenGL::OpenGL )
