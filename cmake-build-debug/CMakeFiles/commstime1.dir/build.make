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
include CMakeFiles/commstime1.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/commstime1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/commstime1.dir/flags.make

CMakeFiles/commstime1.dir/demos/commstime1.cpp.o: CMakeFiles/commstime1.dir/flags.make
CMakeFiles/commstime1.dir/demos/commstime1.cpp.o: ../demos/commstime1.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kevin/usb-encrypted/cpp-csp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/commstime1.dir/demos/commstime1.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/commstime1.dir/demos/commstime1.cpp.o -c /home/kevin/usb-encrypted/cpp-csp/demos/commstime1.cpp

CMakeFiles/commstime1.dir/demos/commstime1.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/commstime1.dir/demos/commstime1.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kevin/usb-encrypted/cpp-csp/demos/commstime1.cpp > CMakeFiles/commstime1.dir/demos/commstime1.cpp.i

CMakeFiles/commstime1.dir/demos/commstime1.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/commstime1.dir/demos/commstime1.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kevin/usb-encrypted/cpp-csp/demos/commstime1.cpp -o CMakeFiles/commstime1.dir/demos/commstime1.cpp.s

CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.requires:

.PHONY : CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.requires

CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.provides: CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.requires
	$(MAKE) -f CMakeFiles/commstime1.dir/build.make CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.provides.build
.PHONY : CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.provides

CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.provides.build: CMakeFiles/commstime1.dir/demos/commstime1.cpp.o


# Object files for target commstime1
commstime1_OBJECTS = \
"CMakeFiles/commstime1.dir/demos/commstime1.cpp.o"

# External object files for target commstime1
commstime1_EXTERNAL_OBJECTS =

commstime1: CMakeFiles/commstime1.dir/demos/commstime1.cpp.o
commstime1: CMakeFiles/commstime1.dir/build.make
commstime1: CMakeFiles/commstime1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kevin/usb-encrypted/cpp-csp/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable commstime1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/commstime1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/commstime1.dir/build: commstime1

.PHONY : CMakeFiles/commstime1.dir/build

CMakeFiles/commstime1.dir/requires: CMakeFiles/commstime1.dir/demos/commstime1.cpp.o.requires

.PHONY : CMakeFiles/commstime1.dir/requires

CMakeFiles/commstime1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/commstime1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/commstime1.dir/clean

CMakeFiles/commstime1.dir/depend:
	cd /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kevin/usb-encrypted/cpp-csp /home/kevin/usb-encrypted/cpp-csp /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug /home/kevin/usb-encrypted/cpp-csp/cmake-build-debug/CMakeFiles/commstime1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/commstime1.dir/depend

