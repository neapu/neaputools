cmake_minimum_required(VERSION 3.15)
set(EXE_NAME NeapuBase)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_definitions(NEAPU_BASE_EXPORTS)
endif()
file(GLOB HEAD_FILES "./*.h")
add_library(${EXE_NAME} SHARED 
	"base_pub.h"
	"NEByteArray.h"
	"NEByteArray.cpp"
	"NELogger.h"
	"NELogger.cpp"
	"NESafeQueue.h"
	"NEString.h"
	"NEThreadPoll.h"
 	"NEUtil.h" 
	"NEUtil.cpp" 
	"NEString.cpp" 
	"NEFile.h" 
	"NEFile.cpp" 
	"NEStringStream.h" 
	"NEStringStream.cpp" 
	"NEDateTime.h" 
	"NEDateTime.cpp"
	"sha/sha256.h"
	"sha/sha256.c")
install(TARGETS ${EXE_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEAD_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})