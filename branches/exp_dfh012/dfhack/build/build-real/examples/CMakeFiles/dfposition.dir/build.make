# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 2.6

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake 2.6\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake 2.6\bin\cmake.exe" -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = "C:\Program Files\CMake 2.6\bin\CMakeSetup.exe"

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real"

# Include any dependencies generated for this target.
include examples/CMakeFiles/dfposition.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/dfposition.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/dfposition.dir/flags.make

examples/CMakeFiles/dfposition.dir/position.cpp.obj: examples/CMakeFiles/dfposition.dir/flags.make
examples/CMakeFiles/dfposition.dir/position.cpp.obj: ../../examples/position.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real\CMakeFiles" $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object examples/CMakeFiles/dfposition.dir/position.cpp.obj"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\examples && c:\MinGW\bin\g++.exe   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles\dfposition.dir\position.cpp.obj -c "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\examples\position.cpp"

examples/CMakeFiles/dfposition.dir/position.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dfposition.dir/position.cpp.i"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\examples && c:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_FLAGS) -E "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\examples\position.cpp" > CMakeFiles\dfposition.dir\position.cpp.i

examples/CMakeFiles/dfposition.dir/position.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dfposition.dir/position.cpp.s"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\examples && c:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_FLAGS) -S "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\examples\position.cpp" -o CMakeFiles\dfposition.dir\position.cpp.s

examples/CMakeFiles/dfposition.dir/position.cpp.obj.requires:
.PHONY : examples/CMakeFiles/dfposition.dir/position.cpp.obj.requires

examples/CMakeFiles/dfposition.dir/position.cpp.obj.provides: examples/CMakeFiles/dfposition.dir/position.cpp.obj.requires
	$(MAKE) -f examples\CMakeFiles\dfposition.dir\build.make examples/CMakeFiles/dfposition.dir/position.cpp.obj.provides.build
.PHONY : examples/CMakeFiles/dfposition.dir/position.cpp.obj.provides

examples/CMakeFiles/dfposition.dir/position.cpp.obj.provides.build: examples/CMakeFiles/dfposition.dir/position.cpp.obj
.PHONY : examples/CMakeFiles/dfposition.dir/position.cpp.obj.provides.build

# Object files for target dfposition
dfposition_OBJECTS = \
"CMakeFiles/dfposition.dir/position.cpp.obj"

# External object files for target dfposition
dfposition_EXTERNAL_OBJECTS =

../../output/dfposition.exe: examples/CMakeFiles/dfposition.dir/position.cpp.obj
../../output/dfposition.exe: ../../output/libdfhack.dll.a
../../output/dfposition.exe: examples/CMakeFiles/dfposition.dir/build.make
../../output/dfposition.exe: examples/CMakeFiles/dfposition.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ..\..\..\output\dfposition.exe"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\dfposition.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/dfposition.dir/build: ../../output/dfposition.exe
.PHONY : examples/CMakeFiles/dfposition.dir/build

examples/CMakeFiles/dfposition.dir/requires: examples/CMakeFiles/dfposition.dir/position.cpp.obj.requires
.PHONY : examples/CMakeFiles/dfposition.dir/requires

examples/CMakeFiles/dfposition.dir/clean:
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\examples && $(CMAKE_COMMAND) -P CMakeFiles\dfposition.dir\cmake_clean.cmake
.PHONY : examples/CMakeFiles/dfposition.dir/clean

examples/CMakeFiles/dfposition.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\examples" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real\examples" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real\examples\CMakeFiles\dfposition.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : examples/CMakeFiles/dfposition.dir/depend

