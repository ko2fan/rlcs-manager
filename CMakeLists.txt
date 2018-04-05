cmake_minimum_required(VERSION 3.9)
project(rlcs_manager C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories("jsmn")
link_directories("jsmn")

set(CMAKE_C_STANDARD 11)

add_executable(rlcs_manager main.c nuklear.h nuklear_sdl_gl3.h)

target_link_libraries(rlcs_manager ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} "jsmn.a")