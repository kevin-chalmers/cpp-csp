# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

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
CMAKE_COMMAND = /usr/local/clion/bin/cmake/bin/cmake

# The command to remove a file.
RM = /usr/local/clion/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kevin/usb-encrypted/cpp-csp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/commstime3.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/commstime3.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/commstime3.dir/flags.make

CMakeFiles/commstime3.dir/demos/commstime3.cpp.o: CMakeFiles/commstime3.dir/flags.make
CMakeFiles/commstime3.dir/demos/commstime3.cpp.o: ../demos/commstime3.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kevin/usb-encrypted/cpp-csp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/commstime3.dir/demos/commstime3.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/commstime3.dir/demos/commstime3.cpp.o -c /home/kevin/usb-encrypted/cpp-csp/demos/commstime3.cpp

CMakeFiles/commstime3.dir/demos/commstime3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/commstime3.dir/demos/commstime3.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kevin/usb-encrypted/cpp-csp/demos/commstime3.cpp > CMakeFiles/commstime3.dir/demos/commstime3.cpp.i

CMakeFiles/commstime3.dir/demos/commstime3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/commstime3.dir/demos/commstime3.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kevin/usb-encrypted/cpp-csp/demos/commstime3.cpp -o CMakeFiles/commstime3.dir/demos/commstime3.cpp.s

CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.requires:

.PHONY : CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.requires

CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.provides: CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.requires
	$(MAKE) -f CMakeFiles/commstime3.dir/build.make CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.provides.build
.PHONY : CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.provides

CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.provides.build: CMakeFiles/commstime3.dir/demos/commstime3.cpp.o


# Object files for target commstime3
commstime3_OBJECTS = \
"CMakeFiles/commstime3.dir/demos/commstime3.cpp.o"

# External object files for target commstime3
commstime3_EXTERNAL_OBJECTS =

commstime3: CMakeFiles/commstime3.dir/demos/commstime3.cpp.o
commstime3: CMakeFiles/commstime3.dir/build.make
commstime3: CMakeFiles/commstime3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kevin/usb-encrypted/cpp-csp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable commstime3"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/commstime3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/commstime3.dir/build: commstime3

.PHONY : CMakeFiles/commstime3.dir/build

CMakeFiles/commstime3.dir/requires: CMakeFiles/commstime3.dir/demos/commstime3.cpp.o.requires

.PHONY : CMakeFiles/commstime3.dir/requires

CMakeFiles/commstime3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/commstime3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/commstime3.dir/clean

CMakeFiles/commstime3.dir/depend:
	cd /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kevin/usb-encrypted/cpp-csp /home/kevin/usb-encrypted/cpp-csp /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug/CMakeFiles/commstime3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/commstime3.dir/depend

