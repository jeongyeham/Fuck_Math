cmake_minimum_required(VERSION 3.10.0)
project(Fuck_Math VERSION 0.1.0 LANGUAGES C)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Choices for compiling
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

# Add the source files
file(GLOB_RECURSE SOURCES "rc/*.*" "src/*.*" "libharu/*.*" "libpng/*.*")
message(STATUS "Source file(s) OK!!!")

# Add the include and lib files
include_directories(libharu/inc libpng/inc zlib/inc rc/)
link_directories(zlib)
message(STATUS "Include and Lib file(s) OK!!!")

# Print map file
add_link_options()

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} libzlibstatic.a)
