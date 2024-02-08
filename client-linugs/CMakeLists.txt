cmake_minimum_required(VERSION 3.7)

project(ClipboardShare) # Name of the project, doesnt mean much
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_BUILD_TYPE Debug)

cmake_policy(SET CMP0037 OLD) # Ignore this, it does some shit to allow /'s in build location names



set(BuildLocation ./build/clipboardshare.o) # Set is a way to define a variable, set(variablename variablecontents)

set(SourceDir ./source/*.cpp)  



file(GLOB_RECURSE SourceFiles ${SourceDir}) # File can manipulate and read files in a directory, the option GLOB_RECURSE goes through them all, the values it finds is stored in SourceFiles and it finds these files in SourceDIR, defined above



add_executable(${BuildLocation} ${SourceFiles}) # Builds source to the location 
target_link_libraries(${BuildLocation}  stdc++fs)  # link to libstdc++fs
