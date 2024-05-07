cmake_minimum_required(VERSION 3.25)
set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
project(krg C CXX)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_executable(entity entity.cc)
target_link_libraries(entity project_options)

add_executable(animation animation.cc)
target_link_libraries(animation project_options)
