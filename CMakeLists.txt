cmake_minimum_required(VERSION 3.15)
project(volconv_all LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ————————————————————————————
# 1) vendored or system OpenVDB
# ————————————————————————————
find_package(OpenVDB QUIET CONFIG)
if(OpenVDB_FOUND)
  message(STATUS "Using system OpenVDB from ${OpenVDB_DIR}")
  set(OPENVDB_LIB openvdb::openvdb)
else()
  message(STATUS "Falling back to vendored OpenVDB in ext/")
  add_subdirectory(ext ext_build)
  set(OPENVDB_LIB openvdb_static)
endif()

# ————————————————————————————
# 2) volconv library
# ————————————————————————————
add_library(volconv_lib STATIC
  volconv.cpp
  volconv.hpp
)
target_link_libraries(volconv_lib PUBLIC ${OPENVDB_LIB})

# ————————————————————————————
# 3) CLI executable
# ————————————————————————————
add_executable(convertvdb main.cpp)
target_link_libraries(convertvdb PRIVATE volconv_lib)

# ————————————————————————————
# 4) Python extension
# ————————————————————————————
find_package(pybind11 REQUIRED)
pybind11_add_module(_volconv
  pybind_volconv.cpp
)
target_link_libraries(_volconv PRIVATE volconv_lib)
