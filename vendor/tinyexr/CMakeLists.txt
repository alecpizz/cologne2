cmake_minimum_required(VERSION 3.5)
project(tinyexr)

add_library(tinyexr)
target_sources(tinyexr PRIVATE "src/tinyexr")
target_include_directories(tinyexr PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

