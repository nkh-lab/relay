cmake_minimum_required(VERSION 3.5)

project(mdl_relay LANGUAGES C)

find_package (Threads)

include_directories(include)

file(GLOB SRC_FILES
    "src/*.c"
    "include/*.h")

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

