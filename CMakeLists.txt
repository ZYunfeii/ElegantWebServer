PROJECT(ElegantWebServer)
cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE HEADER_FILES code/*.h) # 递归头文件

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

file(GLOB_RECURSE SRC_LIST code/*.cpp) # 递归地将code下所有子文件夹cpp文件加入SRC_LIST

message(STATUS ${SRC_LIST})

add_executable(server ${SRC_LIST})

target_link_libraries(server pthread crypto hiredis)