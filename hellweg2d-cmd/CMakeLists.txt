cmake_minimum_required(VERSION 3.6)

project(Hellweg2dCmd)

set(CMAKE_BINARY_DIR       ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib)

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(hellweg2d-cmd ${PROJECT_SOURCE_DIR}/hellweg2d.cpp)

include_directories (kernel)
add_subdirectory (kernel) 

target_link_libraries (Hellweg2dCmd Kernel)
