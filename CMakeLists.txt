cmake_minimum_required(VERSION 3.15)
project(neaputools VERSION 0.0.2)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
message("library dir:" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message("executable dir:" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(/utf-8)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-g)
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
message("install path:" ${CMAKE_INSTALL_PREFIX})
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include/neaputools)

# set(CMAKE_INSTALL_PACKAGEDIR ${CMAKE_INSTALL_LIBDIR}/cmake)
# uninstall target
add_subdirectory(base)
add_subdirectory(network)
add_subdirectory(http)
add_subdirectory(redis_connector)
add_subdirectory(logger)

if(NEAPU_BUILD_DEMO)
  add_subdirectory(demo)
endif()
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
configure_file(
    "neapu-config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/neapu-config.h"
)