cmake_minimum_required(VERSION 3.15)
project(volumeconverter LANGUAGES CXX)

# ————————————————————————————
# 1) Build settings
# ————————————————————————————
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler warnings
if (MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter -march=native -O3 -DNDEBUG)
endif()

# ————————————————————————————
# 2) Locate or vendor in OpenVDB
# ————————————————————————————
# Try to find a system-installed OpenVDB (via OpenVDBConfig.cmake)
find_package(OpenVDB QUIET CONFIG)

if (OpenVDB_FOUND)
  message(STATUS "Using system OpenVDB from ${OpenVDB_DIR}")
  # brings in openvdb::openvdb target, plus Boost, TBB, etc.
  set(OPENVDB_TARGET openvdb::openvdb)
else()
  message(STATUS "Falling back to vendored OpenVDB source in ext/")
  # assume ext contains a cloned OpenVDB repo with its own CMakeLists
  add_subdirectory(ext ext_build)
  # openvdb_static is defined by ext’s CMake
  set(OPENVDB_TARGET openvdb_static)
endif()

# ————————————————————————————
# 3) Your executable
# ————————————————————————————
add_executable(convertvdb main.cpp)

# Include directories only if vendored (system find_package target already handles includes)
if (NOT OpenVDB_FOUND)
  target_include_directories(convertvdb PRIVATE ${OPENVDB_INCLUDE_DIR})
endif()

# Link to the proper OpenVDB target
target_link_libraries(convertvdb PRIVATE ${OPENVDB_TARGET})

# ————————————————————————————
# 4) Usage hints (optional)
# ————————————————————————————
if (NOT OpenVDB_FOUND)
  message(STATUS "To use a system-installed OpenVDB, rerun CMake with:\n"
         "  -DOpenVDB_DIR=/path/to/OpenVDBConfig.cmake")
endif()
