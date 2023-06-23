cmake_minimum_required(VERSION 3.25)
project(server_thing)

set(CMAKE_CXX_STANDARD 23)

add_executable(server_thing main.cpp manifest_info.hpp )

if (WIN32)
    add_executable(server_thing)
endif ()
target_link_libraries(${PROJECT_NAME} cpr curl simdjson)