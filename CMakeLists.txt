cmake_minimum_required(VERSION 3.31)
project(connector)

set(CMAKE_CXX_STANDARD 26)

add_executable(connector core.cpp ui.cpp)
target_link_libraries(connector PRIVATE dwmapi)