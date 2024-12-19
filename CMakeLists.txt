cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PRJ HttpServer)
set(PRJ_INCLUDES_DIR includes)
set(PRJ_SRC_DIR src)
set(PRJ_SRC_FILES ${PRJ_SRC_DIR}/main.cpp
				  ${PRJ_SRC_DIR}/server.cpp
				  ${PRJ_SRC_DIR}/http_message.cpp
				  ${PRJ_SRC_DIR}/utils.cpp)
project(${PRJ} VERSION 0.1.0)
add_executable(HttpServer ${PRJ_SRC_FILES})
target_include_directories(HttpServer PUBLIC ${PRJ_INCLUDES_DIR})
