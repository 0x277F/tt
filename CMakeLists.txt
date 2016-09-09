cmake_minimum_required(VERSION 3.5)
project(tt)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
FIND_PACKAGE(X11 REQUIRED)
if (X11_FOUND)
    INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
    LINK_LIBRARIES(${X11_LIBRARIES})
ENDIF(X11_FOUND)

set(SOURCE_FILES main.c main.c tt.c tt.h)
add_executable(tt ${SOURCE_FILES})