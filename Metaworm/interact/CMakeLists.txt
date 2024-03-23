cmake_minimum_required(VERSION 2.8.6)
project(interact)

add_compile_options(-fPIC)
add_compile_options(-std=gnu++11)
add_compile_options(-Wdeprecated-declarations)

set(BOOST_ROOT "/usr/local")
find_package(Boost #[[1.66.0]] COMPONENTS python3 numpy3 filesystem)
find_package(PythonLibs REQUIRED)
find_package(Eigen3 REQUIRED)

link_directories(../sim/)
include_directories(../sim/)
link_directories(../sim/fem)
include_directories(../sim/fem)
link_directories(../learn)
include_directories(../learn)
include_directories(../../neuronXcore)
link_directories(../../neuronXcore)
include_directories(../render)
include_directories(../../owl/samples/common/3rdParty/glfw/include)

file(GLOB srcs "*.h" "*.cpp" "*.hpp")
include_directories(${PYTHON_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${TinyXML_INCLUDE_DIRS})


include(FindOpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_library(interact SHARED ${srcs})
target_link_libraries(interact ${Boost_LIBRARIES} ${TinyXML_LIBRARIES} ${PYTHON_LIBRARIES} sim fem)
set_target_properties(interact PROPERTIES PREFIX "" )

set(WORM_INTERACT_INCLUDES
    ${PROJECT_SOURCE_DIR} PARENT_SCOPE)
