# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lyslg/Documents/lyslg_/test_function

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lyslg/Documents/lyslg_/test_function/build

# Include any dependencies generated for this target.
include CMakeFiles/test_chartostr.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_chartostr.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_chartostr.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_chartostr.dir/flags.make

CMakeFiles/test_chartostr.dir/test_chartostr.cc.o: CMakeFiles/test_chartostr.dir/flags.make
CMakeFiles/test_chartostr.dir/test_chartostr.cc.o: ../test_chartostr.cc
CMakeFiles/test_chartostr.dir/test_chartostr.cc.o: CMakeFiles/test_chartostr.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lyslg/Documents/lyslg_/test_function/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_chartostr.dir/test_chartostr.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_chartostr.dir/test_chartostr.cc.o -MF CMakeFiles/test_chartostr.dir/test_chartostr.cc.o.d -o CMakeFiles/test_chartostr.dir/test_chartostr.cc.o -c /home/lyslg/Documents/lyslg_/test_function/test_chartostr.cc

CMakeFiles/test_chartostr.dir/test_chartostr.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_chartostr.dir/test_chartostr.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lyslg/Documents/lyslg_/test_function/test_chartostr.cc > CMakeFiles/test_chartostr.dir/test_chartostr.cc.i

CMakeFiles/test_chartostr.dir/test_chartostr.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_chartostr.dir/test_chartostr.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lyslg/Documents/lyslg_/test_function/test_chartostr.cc -o CMakeFiles/test_chartostr.dir/test_chartostr.cc.s

# Object files for target test_chartostr
test_chartostr_OBJECTS = \
"CMakeFiles/test_chartostr.dir/test_chartostr.cc.o"

# External object files for target test_chartostr
test_chartostr_EXTERNAL_OBJECTS =

../bin/test_chartostr: CMakeFiles/test_chartostr.dir/test_chartostr.cc.o
../bin/test_chartostr: CMakeFiles/test_chartostr.dir/build.make
../bin/test_chartostr: CMakeFiles/test_chartostr.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lyslg/Documents/lyslg_/test_function/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test_chartostr"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_chartostr.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_chartostr.dir/build: ../bin/test_chartostr
.PHONY : CMakeFiles/test_chartostr.dir/build

CMakeFiles/test_chartostr.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_chartostr.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_chartostr.dir/clean

CMakeFiles/test_chartostr.dir/depend:
	cd /home/lyslg/Documents/lyslg_/test_function/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lyslg/Documents/lyslg_/test_function /home/lyslg/Documents/lyslg_/test_function /home/lyslg/Documents/lyslg_/test_function/build /home/lyslg/Documents/lyslg_/test_function/build /home/lyslg/Documents/lyslg_/test_function/build/CMakeFiles/test_chartostr.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_chartostr.dir/depend

