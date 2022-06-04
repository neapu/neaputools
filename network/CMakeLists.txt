cmake_minimum_required(VERSION 3.15)
set(EXE_NAME NeapuNetwork)
include_directories("../base")
if(WIN32)
	set(LIB_EVENT_INCLUDE "D:/code/libevent/include" CACHE FILEPATH "libevent include path")
	include_directories(${LIB_EVENT_INCLUDE})
	find_library(LIBEVENT_CORE event_core "D:/code/libevent/lib")
else()
	find_library(LIBEVENT_CORE event_core "/usr/lib" "/usr/local/lib")
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_definitions(NEAPU_NETWORK_EXPORTS)
endif()
file(GLOB HEAD_FILES "./*.h")
add_library(${EXE_NAME} SHARED 
	"network_pub.h"
	"NENetChannel.h"
	"NENetChannel.cpp"
	"NETcpServer.h"
	"NETcpServer.cpp"
	
	
 "NETcpClient.h" "NETcpClient.cpp" "NEUdpBase.h" "NEUdpBase.cpp" "NEIPAddress.h" "NEIPAddress.cpp" "NENetBase.h" "NENetBase.cpp" "NENetworkError.cpp"  )
target_link_libraries(${EXE_NAME} PUBLIC NeapuBase ${LIBEVENT_CORE})
if(WIN32)
target_link_libraries(${EXE_NAME} PUBLIC ws2_32)
endif()
install(TARGETS ${EXE_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEAD_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})