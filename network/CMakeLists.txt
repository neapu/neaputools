cmake_minimum_required(VERSION 3.15)
project(neapunetwork CXX)
include_directories("../base")
if(WIN32)
	include_directories("D:/code/libevent/include")
	find_library(LIBEVENT_CORE event_core "D:/code/libevent/lib")
else()
	find_library(LIBEVENT_CORE event_core "/usr/lib" "/usr/local/lib")
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_definitions(NEAPU_NETWORK_EXPORTS)
endif()
add_library(${PROJECT_NAME} SHARED 
	"network_pub.h"
	"NENetChannel.h"
	"NENetChannel.cpp"
	"NETcpBase.h"
	"NETcpBase.cpp"
	"NENetWorkThread.h"
	"NENetWorkThread.cpp"
 "NETcpClient.h" "NETcpClient.cpp")
target_link_libraries(${PROJECT_NAME} PUBLIC neapubase ${LIBEVENT_CORE})
if(WIN32)
target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32)
endif()
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEAD_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})