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
include tools/CMakeFiles/dfitemdesignator.dir/depend.make

# Include the progress variables for this target.
include tools/CMakeFiles/dfitemdesignator.dir/progress.make

# Include the compile flags for this target's objects.
include tools/CMakeFiles/dfitemdesignator.dir/flags.make

tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj: tools/CMakeFiles/dfitemdesignator.dir/flags.make
tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj: ../../tools/itemdesignator.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real\CMakeFiles" $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\tools && c:\MinGW\bin\g++.exe   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles\dfitemdesignator.dir\itemdesignator.cpp.obj -c "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\tools\itemdesignator.cpp"

tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.i"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\tools && c:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_FLAGS) -E "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\tools\itemdesignator.cpp" > CMakeFiles\dfitemdesignator.dir\itemdesignator.cpp.i

tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.s"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\tools && c:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_FLAGS) -S "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\tools\itemdesignator.cpp" -o CMakeFiles\dfitemdesignator.dir\itemdesignator.cpp.s

tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.requires:
.PHONY : tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.requires

tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.provides: tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.requires
	$(MAKE) -f tools\CMakeFiles\dfitemdesignator.dir\build.make tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.provides.build
.PHONY : tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.provides

tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.provides.build: tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj
.PHONY : tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.provides.build

# Object files for target dfitemdesignator
dfitemdesignator_OBJECTS = \
"CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj"

# External object files for target dfitemdesignator
dfitemdesignator_EXTERNAL_OBJECTS =

../../output/dfitemdesignator.exe: tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj
../../output/dfitemdesignator.exe: ../../output/libdfhack.dll.a
../../output/dfitemdesignator.exe: tools/CMakeFiles/dfitemdesignator.dir/build.make
../../output/dfitemdesignator.exe: tools/CMakeFiles/dfitemdesignator.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ..\..\..\output\dfitemdesignator.exe"
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\dfitemdesignator.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/CMakeFiles/dfitemdesignator.dir/build: ../../output/dfitemdesignator.exe
.PHONY : tools/CMakeFiles/dfitemdesignator.dir/build

tools/CMakeFiles/dfitemdesignator.dir/requires: tools/CMakeFiles/dfitemdesignator.dir/itemdesignator.cpp.obj.requires
.PHONY : tools/CMakeFiles/dfitemdesignator.dir/requires

tools/CMakeFiles/dfitemdesignator.dir/clean:
	cd D:\dloads\COPYOF~1\PETERI~1\build\BUILD-~1\tools && $(CMAKE_COMMAND) -P CMakeFiles\dfitemdesignator.dir\cmake_clean.cmake
.PHONY : tools/CMakeFiles/dfitemdesignator.dir/clean

tools/CMakeFiles/dfitemdesignator.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\tools" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real\tools" "D:\dloads\Copy of peterix-dfhack-0ab103d\peterix-dfhack-0ab103d\build\build-real\tools\CMakeFiles\dfitemdesignator.dir\DependInfo.cmake" --color=$(COLOR)
.PHONY : tools/CMakeFiles/dfitemdesignator.dir/depend

