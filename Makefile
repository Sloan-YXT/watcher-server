# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /home/yaoxuetao/CMPW-server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yaoxuetao/CMPW-server

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/yaoxuetao/CMPW-server/CMakeFiles /home/yaoxuetao/CMPW-server/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/yaoxuetao/CMPW-server/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named server

# Build rule for target.
server: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 server
.PHONY : server

# fast build rule for target.
server/fast:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/build
.PHONY : server/fast

cutil.o: cutil.cpp.o

.PHONY : cutil.o

# target to build an object file
cutil.cpp.o:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/cutil.cpp.o
.PHONY : cutil.cpp.o

cutil.i: cutil.cpp.i

.PHONY : cutil.i

# target to preprocess a source file
cutil.cpp.i:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/cutil.cpp.i
.PHONY : cutil.cpp.i

cutil.s: cutil.cpp.s

.PHONY : cutil.s

# target to generate assembly for a file
cutil.cpp.s:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/cutil.cpp.s
.PHONY : cutil.cpp.s

database.o: database.cpp.o

.PHONY : database.o

# target to build an object file
database.cpp.o:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/database.cpp.o
.PHONY : database.cpp.o

database.i: database.cpp.i

.PHONY : database.i

# target to preprocess a source file
database.cpp.i:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/database.cpp.i
.PHONY : database.cpp.i

database.s: database.cpp.s

.PHONY : database.s

# target to generate assembly for a file
database.cpp.s:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/database.cpp.s
.PHONY : database.cpp.s

faceDetect.o: faceDetect.cpp.o

.PHONY : faceDetect.o

# target to build an object file
faceDetect.cpp.o:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/faceDetect.cpp.o
.PHONY : faceDetect.cpp.o

faceDetect.i: faceDetect.cpp.i

.PHONY : faceDetect.i

# target to preprocess a source file
faceDetect.cpp.i:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/faceDetect.cpp.i
.PHONY : faceDetect.cpp.i

faceDetect.s: faceDetect.cpp.s

.PHONY : faceDetect.s

# target to generate assembly for a file
faceDetect.cpp.s:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/faceDetect.cpp.s
.PHONY : faceDetect.cpp.s

server.o: server.cpp.o

.PHONY : server.o

# target to build an object file
server.cpp.o:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server.cpp.o
.PHONY : server.cpp.o

server.i: server.cpp.i

.PHONY : server.i

# target to preprocess a source file
server.cpp.i:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server.cpp.i
.PHONY : server.cpp.i

server.s: server.cpp.s

.PHONY : server.s

# target to generate assembly for a file
server.cpp.s:
	$(MAKE) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server.cpp.s
.PHONY : server.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... server"
	@echo "... edit_cache"
	@echo "... cutil.o"
	@echo "... cutil.i"
	@echo "... cutil.s"
	@echo "... database.o"
	@echo "... database.i"
	@echo "... database.s"
	@echo "... faceDetect.o"
	@echo "... faceDetect.i"
	@echo "... faceDetect.s"
	@echo "... server.o"
	@echo "... server.i"
	@echo "... server.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

