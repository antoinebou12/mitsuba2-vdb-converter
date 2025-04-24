# ext/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# 1) Make sure the OpenVDB submodule is here
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/openvdb/CMakeLists.txt")
  message(FATAL_ERROR
    "OpenVDB submodule not found!\n"
    "Please clone with --recursive or run:\n"
    "  git submodule update --init --recursive")
endif()

# 2) Pick a C++ standard (14 if available, else 11)
include(CheckCXXCompilerFlag)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11)
  if (HAS_CPP14)
    set(CMAKE_CXX_STANDARD 14 CACHE STRING "" FORCE)
  elseif (HAS_CPP11)
    set(CMAKE_CXX_STANDARD 11 CACHE STRING "" FORCE)
  endif()
endif()

# 3) Compiler warning flags
if (MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  add_compile_options(/MP /W4)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# 4) Build vendored OpenVDB (but don’t drag its tests/examples into your IDE)
add_subdirectory(openvdb EXCLUDE_FROM_ALL)

# 5) Export include path and library target to the parent CMake
set(OPENVDB_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/openvdb"
    PARENT_SCOPE)

# the OpenVDB CMake defines a target called `openvdb_static`
set(OPENVDB_LIBRARY
    openvdb_static
    PARENT_SCOPE)
