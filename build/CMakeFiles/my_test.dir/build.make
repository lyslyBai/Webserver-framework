# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /src_temp/cmake_src/cmake-3.9.2/bin/cmake

# The command to remove a file.
RM = /src_temp/cmake_src/cmake-3.9.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lyslg/lyslg_

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lyslg/lyslg_/build

# Include any dependencies generated for this target.
include CMakeFiles/my_test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/my_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/my_test.dir/flags.make

CMakeFiles/my_test.dir/tests/test.cc.o: CMakeFiles/my_test.dir/flags.make
CMakeFiles/my_test.dir/tests/test.cc.o: ../tests/test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lyslg/lyslg_/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/my_test.dir/tests/test.cc.o"
	/opt/rh/devtoolset-8/root/usr/bin/c++  $(CXX_DEFINES) -D__FILE__=\"tests/test.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_test.dir/tests/test.cc.o -c /home/lyslg/lyslg_/tests/test.cc

CMakeFiles/my_test.dir/tests/test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_test.dir/tests/test.cc.i"
	/opt/rh/devtoolset-8/root/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lyslg/lyslg_/tests/test.cc > CMakeFiles/my_test.dir/tests/test.cc.i

CMakeFiles/my_test.dir/tests/test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_test.dir/tests/test.cc.s"
	/opt/rh/devtoolset-8/root/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lyslg/lyslg_/tests/test.cc -o CMakeFiles/my_test.dir/tests/test.cc.s

CMakeFiles/my_test.dir/tests/test.cc.o.requires:

.PHONY : CMakeFiles/my_test.dir/tests/test.cc.o.requires

CMakeFiles/my_test.dir/tests/test.cc.o.provides: CMakeFiles/my_test.dir/tests/test.cc.o.requires
	$(MAKE) -f CMakeFiles/my_test.dir/build.make CMakeFiles/my_test.dir/tests/test.cc.o.provides.build
.PHONY : CMakeFiles/my_test.dir/tests/test.cc.o.provides

CMakeFiles/my_test.dir/tests/test.cc.o.provides.build: CMakeFiles/my_test.dir/tests/test.cc.o


# Object files for target my_test
my_test_OBJECTS = \
"CMakeFiles/my_test.dir/tests/test.cc.o"

# External object files for target my_test
my_test_EXTERNAL_OBJECTS =

../bin/my_test: CMakeFiles/my_test.dir/tests/test.cc.o
../bin/my_test: CMakeFiles/my_test.dir/build.make
../bin/my_test: ../lib/liblyslg.so
../bin/my_test: CMakeFiles/my_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lyslg/lyslg_/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/my_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/my_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/my_test.dir/build: ../bin/my_test

.PHONY : CMakeFiles/my_test.dir/build

CMakeFiles/my_test.dir/requires: CMakeFiles/my_test.dir/tests/test.cc.o.requires

.PHONY : CMakeFiles/my_test.dir/requires

CMakeFiles/my_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/my_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/my_test.dir/clean

CMakeFiles/my_test.dir/depend:
	cd /home/lyslg/lyslg_/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lyslg/lyslg_ /home/lyslg/lyslg_ /home/lyslg/lyslg_/build /home/lyslg/lyslg_/build /home/lyslg/lyslg_/build/CMakeFiles/my_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/my_test.dir/depend

