# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/brains/worm_simulation/interact/worm-simulation/neuronXcore

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/brains/worm_simulation/interact/worm-simulation/build2

# Include any dependencies generated for this target.
include external_owl/owl/common/CMakeFiles/owl-common.dir/depend.make

# Include the progress variables for this target.
include external_owl/owl/common/CMakeFiles/owl-common.dir/progress.make

# Include the compile flags for this target's objects.
include external_owl/owl/common/CMakeFiles/owl-common.dir/flags.make

external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o: external_owl/owl/common/CMakeFiles/owl-common.dir/flags.make
external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o: /home/brains/worm_simulation/interact/worm-simulation/owl/owl/common/owl-common.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/brains/worm_simulation/interact/worm-simulation/build2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o"
	cd /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/owl-common.dir/owl-common.cpp.o -c /home/brains/worm_simulation/interact/worm-simulation/owl/owl/common/owl-common.cpp

external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/owl-common.dir/owl-common.cpp.i"
	cd /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/brains/worm_simulation/interact/worm-simulation/owl/owl/common/owl-common.cpp > CMakeFiles/owl-common.dir/owl-common.cpp.i

external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/owl-common.dir/owl-common.cpp.s"
	cd /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/brains/worm_simulation/interact/worm-simulation/owl/owl/common/owl-common.cpp -o CMakeFiles/owl-common.dir/owl-common.cpp.s

external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.requires:

.PHONY : external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.requires

external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.provides: external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.requires
	$(MAKE) -f external_owl/owl/common/CMakeFiles/owl-common.dir/build.make external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.provides.build
.PHONY : external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.provides

external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.provides.build: external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o


# Object files for target owl-common
owl__common_OBJECTS = \
"CMakeFiles/owl-common.dir/owl-common.cpp.o"

# External object files for target owl-common
owl__common_EXTERNAL_OBJECTS =

external_owl/libowl-common.a: external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o
external_owl/libowl-common.a: external_owl/owl/common/CMakeFiles/owl-common.dir/build.make
external_owl/libowl-common.a: external_owl/owl/common/CMakeFiles/owl-common.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/brains/worm_simulation/interact/worm-simulation/build2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../../libowl-common.a"
	cd /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common && $(CMAKE_COMMAND) -P CMakeFiles/owl-common.dir/cmake_clean_target.cmake
	cd /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/owl-common.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
external_owl/owl/common/CMakeFiles/owl-common.dir/build: external_owl/libowl-common.a

.PHONY : external_owl/owl/common/CMakeFiles/owl-common.dir/build

external_owl/owl/common/CMakeFiles/owl-common.dir/requires: external_owl/owl/common/CMakeFiles/owl-common.dir/owl-common.cpp.o.requires

.PHONY : external_owl/owl/common/CMakeFiles/owl-common.dir/requires

external_owl/owl/common/CMakeFiles/owl-common.dir/clean:
	cd /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common && $(CMAKE_COMMAND) -P CMakeFiles/owl-common.dir/cmake_clean.cmake
.PHONY : external_owl/owl/common/CMakeFiles/owl-common.dir/clean

external_owl/owl/common/CMakeFiles/owl-common.dir/depend:
	cd /home/brains/worm_simulation/interact/worm-simulation/build2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/brains/worm_simulation/interact/worm-simulation/neuronXcore /home/brains/worm_simulation/interact/worm-simulation/owl/owl/common /home/brains/worm_simulation/interact/worm-simulation/build2 /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common /home/brains/worm_simulation/interact/worm-simulation/build2/external_owl/owl/common/CMakeFiles/owl-common.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : external_owl/owl/common/CMakeFiles/owl-common.dir/depend

