   cmake_minimum_required(VERSION 3.10)
   project(SimDB VERSION 1.0)

   # Set the C++ standard
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   # Add all source files
   file(GLOB_RECURSE SOURCES "src/*.cpp")

   # Create the executable with all source files
   add_executable(SimDB ${SOURCES})

   # Include directories
   target_include_directories(SimDB PRIVATE src)

   # Make sure data directory exists
   file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)