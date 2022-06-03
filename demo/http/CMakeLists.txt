set(EXE_NAME http_demo)
include_directories("../../base" "../../network" "../../http")
add_executable(${EXE_NAME} "main.cpp"  )
target_link_libraries(${EXE_NAME} PUBLIC NeapuBase NeapuNetwork NeapuHttp)