cmake_minimum_required(VERSION 3.5)
project(stb_dxt)

add_library(stb_dxt)
target_sources(stb_dxt PRIVATE "src/stb_dxt")
target_include_directories(stb_dxt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

