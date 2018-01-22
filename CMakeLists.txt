cmake_minimum_required(VERSION 2.8.9)
project(myshell)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
add_subdirectory(${myshell_SOURCE_DIR}/src)
