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

#find_package(OpenGL REQUIRED)
#include(configure_glut)

include(FindOpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
cuda_compile_and_embed(embedded_common_programs device/common.cu)
cuda_compile_and_embed(embedded_neurons_programs device/basicNeurons.cu)

#include_directories(${GLUT_INCLUDE_DIR})
add_executable(neuronXcore
  ${embedded_common_programs}
  ${embedded_neurons_programs}
  external/filesystem.hpp
  device/Camera.h
  device/common.h
  device/disney_bsdf.h
  device/FrameState.h
  device/PerRayData.h
  device/roundedCone.h
  device/RayGenData.h
  device/TriangleMesh.h
  device/TubesGeom.h
  Viewer.h
  viewer.cpp
  material.cpp
  TubesUtils.h
  TubesUtils.cpp
  OptixTubes.h
  OptixTubes.cpp
  BasicNeurons.h
  BasicNeurons.cpp
  Triangles.h
  Triangles.cpp
  profiler.h
  profiler.cpp
  CmdlineParser.h
  CmdlineParser.cpp
  CameraTrack.h
  CameraTrack.cpp
  CameraMode.h
  CameraMode.cpp
  Curve.h
  Curve.cpp
  neuronXcore.cpp
  )

target_link_libraries(neuronXcore
  ${OWL_VIEWER_LIBRARIES}
  ${OWL_LIBRARIES}
  imgui
  render
  sim
  $<$<STREQUAL:${GLEW_LIBRARIES},"">:GLEW::glew_s>
  ${GLEW_LIBRARIES}
  ${Boost_LIBRARIES} 
  ${PYTHON_LIBRARIES}
  ${GLUT_LIBRARIES}
  fem
  interact
  )
  
 if (VERBOSE_MODE)
	message(STATUS "===>verbose")
	message(STATUS "OWL_VIEWER_LIBRARIES='${OWL_VIEWER_LIBRARIES}'")
	message(STATUS "OWL_LIBRARIES='${OWL_LIBRARIES}'")
	message(STATUS "===>verbose")
endif (VERBOSE_MODE)
