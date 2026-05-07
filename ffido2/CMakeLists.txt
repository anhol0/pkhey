cmake_minimum_required(VERSION 3.20)
project(ffido CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(ffido
    src/ffido.cpp
)
target_compile_options(ffido PRIVATE -g)
target_link_libraries(ffido )
