SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.4.0)



project(ubitrrack_sinkgroup_test VERSION 1.3.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(GNUInstallDirs)
include(cmake/UbitrackConfig)



# External Libraries (could this be solved with conan dependencies ??)

# OpenGL
SET(HAVE_OPENGL 0)
find_package(OpenGL)
IF(OpenGL_FOUND)
  SET(HAVE_OPENGL 1)
  MESSAGE(STATUS "Found OpenGL: ${OpenGL_INCLUDE_DIR} - ${OpenGL_LIBRARIES}")
ENDIF(OpenGL_FOUND)

# OpenCL
SET(HAVE_OPENCL 0)
find_package(OpenCL)
IF(OpenCL_FOUND)
  include_directories(${OpenCL_INCLUDE_DIR})
  SET(HAVE_OPENCL 1)
  MESSAGE(STATUS "Found OpenCL: ${OpenCL_INCLUDE_DIR} - ${OpenCL_LIBRARY}")
ENDIF(OpenCL_FOUND)


# External Libraries (could this be solved with conan dependencies ??)

IF(ENABLE_BASICFACADE)
  set(UBITRACK_COMPILE_DEFINITIONS ${UBITRACK_COMPILE_DEFINITIONS} "ENABLE_BASICFACADE")
ENDIF(ENABLE_BASICFACADE)

# wrappers and helpers
add_subdirectory(sinkgroup_test)
