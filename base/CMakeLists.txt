cmake_minimum_required(VERSION 3.15)
project(neapubase CXX)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_definitions(NEAPU_BASE_EXPORTS)
endif()
add_library(${PROJECT_NAME} SHARED 
	"base_pub.h"
	"NEByteArray.h"
	"NEByteArray.cpp"
	"NELogger.h"
	"NELogger.cpp"
	"NESafeQueue.h"
	"NEString.h"
	"NEThreadPoll.h"
 "NEUtil.h" "NEUtil.cpp" "NEString.cpp")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEAD_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})