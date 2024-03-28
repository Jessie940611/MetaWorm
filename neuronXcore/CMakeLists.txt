# ======================================================================== #
# Copyright 2018-2020 The Contributors                                     #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #

cmake_policy(SET CMP0048 NEW)
project(msduo VERSION 0.1.0)

cmake_minimum_required(VERSION 2.8)
if (NOT WIN32)
set (CMAKE_CXX_FLAGS "--std=c++11")
set (CUDA_PROPAGATE_HOST_FLAGS ON)
endif()

option(VERBOSE_MODE "Turn Verbose on to output debug info." ON)

set(owl_dir ${CMAKE_CURRENT_SOURCE_DIR}/../owl)
set(metaworm_dir ${CMAKE_CURRENT_SOURCE_DIR}/../Metaworm)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${owl_dir}/owl/common/cmake/")

include(configure_cuda)
include(configure_owl)

set(BIN2C ${CUDA_TOOLKIT_ROOT_DIR}/bin/bin2c)

#include(configure_optix)
#include(configure_build_type)

#find_package(TBB REQUIRED)
#include_directories(${TBB_INCLUDE_DIRS})

# -------------------------------------------------------
if (NOT WIN32) 
set(CMAKE_CXX_FLAGS "-std=c++11")
endif()
if(WIN32)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099 /ignore:4819")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")
endif()


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
# -------------------------------------------------------

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/glad/include")

add_subdirectory(ext/imgui external_imgui EXCLUDE_FROM_ALL)
  
add_subdirectory(${owl_dir} external_owl EXCLUDE_FROM_ALL)

message(STATUS "============build target='${owl_dir}' done============")

include_directories("${OWL_INCLUDES}")

add_subdirectory(${owl_dir}/samples/common/owlViewer external_owl_viewer EXCLUDE_FROM_ALL)
message(STATUS "============build target='${owl_dir}/samples/common/owlViewer' done============")

include_directories("${OWL_VIEWER_INCLUDES}")


# ---------------------------------------------------------
add_subdirectory(${metaworm_dir} metaworm EXCLUDE_FROM_ALL)

include_directories("${META_WORM_INCLUDES}")

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

## list include directory for debug
if (VERBOSE_MODE)
	get_property(includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
	message(STATUS "===>verbose")
	foreach(dir ${includes})
		message(STATUS "include dir='${dir}'")
	endforeach()
	message(STATUS "===>verbose")
endif (VERBOSE_MODE)


# -------------------------------------------------------

add_subdirectory(rtn)


#if (TARGET glfw)
#	install(TARGETS glfw imgui llowl_static owl_static owl_viewer neuronXcore)
#else()
#	install(TARGETS imgui llowl_static owl_static owl_viewer neuronXcore)
#endif()
#	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
#	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
	

