# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lifesim/MetaWorm/neuronXcore

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lifesim/MetaWorm/build2

# Include any dependencies generated for this target.
include external_imgui/CMakeFiles/imgui.dir/depend.make

# Include the progress variables for this target.
include external_imgui/CMakeFiles/imgui.dir/progress.make

# Include the compile flags for this target's objects.
include external_imgui/CMakeFiles/imgui.dir/flags.make

external_imgui/CMakeFiles/imgui.dir/imgui.cpp.o: external_imgui/CMakeFiles/imgui.dir/flags.make
external_imgui/CMakeFiles/imgui.dir/imgui.cpp.o: /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lifesim/MetaWorm/build2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object external_imgui/CMakeFiles/imgui.dir/imgui.cpp.o"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/imgui.cpp.o -c /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui.cpp

external_imgui/CMakeFiles/imgui.dir/imgui.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui.cpp.i"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui.cpp > CMakeFiles/imgui.dir/imgui.cpp.i

external_imgui/CMakeFiles/imgui.dir/imgui.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui.cpp.s"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui.cpp -o CMakeFiles/imgui.dir/imgui.cpp.s

external_imgui/CMakeFiles/imgui.dir/imgui_demo.cpp.o: external_imgui/CMakeFiles/imgui.dir/flags.make
external_imgui/CMakeFiles/imgui.dir/imgui_demo.cpp.o: /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_demo.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lifesim/MetaWorm/build2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object external_imgui/CMakeFiles/imgui.dir/imgui_demo.cpp.o"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/imgui_demo.cpp.o -c /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_demo.cpp

external_imgui/CMakeFiles/imgui.dir/imgui_demo.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui_demo.cpp.i"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_demo.cpp > CMakeFiles/imgui.dir/imgui_demo.cpp.i

external_imgui/CMakeFiles/imgui.dir/imgui_demo.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui_demo.cpp.s"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_demo.cpp -o CMakeFiles/imgui.dir/imgui_demo.cpp.s

external_imgui/CMakeFiles/imgui.dir/imgui_draw.cpp.o: external_imgui/CMakeFiles/imgui.dir/flags.make
external_imgui/CMakeFiles/imgui.dir/imgui_draw.cpp.o: /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_draw.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lifesim/MetaWorm/build2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object external_imgui/CMakeFiles/imgui.dir/imgui_draw.cpp.o"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/imgui_draw.cpp.o -c /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_draw.cpp

external_imgui/CMakeFiles/imgui.dir/imgui_draw.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui_draw.cpp.i"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_draw.cpp > CMakeFiles/imgui.dir/imgui_draw.cpp.i

external_imgui/CMakeFiles/imgui.dir/imgui_draw.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui_draw.cpp.s"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_draw.cpp -o CMakeFiles/imgui.dir/imgui_draw.cpp.s

external_imgui/CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.o: external_imgui/CMakeFiles/imgui.dir/flags.make
external_imgui/CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.o: /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_impl_glfw_gl2.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lifesim/MetaWorm/build2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object external_imgui/CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.o"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.o -c /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_impl_glfw_gl2.cpp

external_imgui/CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.i"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_impl_glfw_gl2.cpp > CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.i

external_imgui/CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.s"
	cd /home/lifesim/MetaWorm/build2/external_imgui && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lifesim/MetaWorm/neuronXcore/ext/imgui/imgui_impl_glfw_gl2.cpp -o CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.s

# Object files for target imgui
imgui_OBJECTS = \
"CMakeFiles/imgui.dir/imgui.cpp.o" \
"CMakeFiles/imgui.dir/imgui_demo.cpp.o" \
"CMakeFiles/imgui.dir/imgui_draw.cpp.o" \
"CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.o"

# External object files for target imgui
imgui_EXTERNAL_OBJECTS =

libimgui.so: external_imgui/CMakeFiles/imgui.dir/imgui.cpp.o
libimgui.so: external_imgui/CMakeFiles/imgui.dir/imgui_demo.cpp.o
libimgui.so: external_imgui/CMakeFiles/imgui.dir/imgui_draw.cpp.o
libimgui.so: external_imgui/CMakeFiles/imgui.dir/imgui_impl_glfw_gl2.cpp.o
libimgui.so: external_imgui/CMakeFiles/imgui.dir/build.make
libimgui.so: libglfw.so.3.4
libimgui.so: /usr/lib/x86_64-linux-gnu/libGL.so
libimgui.so: external_imgui/CMakeFiles/imgui.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lifesim/MetaWorm/build2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX shared library ../libimgui.so"
	cd /home/lifesim/MetaWorm/build2/external_imgui && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/imgui.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
external_imgui/CMakeFiles/imgui.dir/build: libimgui.so

.PHONY : external_imgui/CMakeFiles/imgui.dir/build

external_imgui/CMakeFiles/imgui.dir/clean:
	cd /home/lifesim/MetaWorm/build2/external_imgui && $(CMAKE_COMMAND) -P CMakeFiles/imgui.dir/cmake_clean.cmake
.PHONY : external_imgui/CMakeFiles/imgui.dir/clean

external_imgui/CMakeFiles/imgui.dir/depend:
	cd /home/lifesim/MetaWorm/build2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lifesim/MetaWorm/neuronXcore /home/lifesim/MetaWorm/neuronXcore/ext/imgui /home/lifesim/MetaWorm/build2 /home/lifesim/MetaWorm/build2/external_imgui /home/lifesim/MetaWorm/build2/external_imgui/CMakeFiles/imgui.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : external_imgui/CMakeFiles/imgui.dir/depend

