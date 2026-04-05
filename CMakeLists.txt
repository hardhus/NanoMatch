cmake_minimum_required(VERSION 3.16)
project(
  NanoMatch
  VERSION 1.0.0
  LANGUAGES CXX)

if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp)
