project(Nocturne)
cmake_minimum_required(VERSION 3.22)

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

set (CMAKE_SYSROOT /)
set (cross_path $ENV{HOME}/.local/cross)

set (CMAKE_LINKER ${cross_path}/bin/aarch64-rockchip-linux-ld)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_C_COMPILER ${cross_path}/bin/aarch64-rockchip-linux-gcc)
set (CMAKE_CXX_COMPILER ${cross_path}/bin/aarch64-rockchip-linux-g++)

set(CMAKE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)


#lib files
set(libtf $ENV{HOME}/Documents/tflite_build/libtensorflow-lite.a)

#include paths
set(tensorflow_include $ENV{HOME}/Documents/tensorflow_src)

#add_compile_options(-static)

add_executable(nocturne src/test.cpp)
set_target_properties( nocturne PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
) 

target_include_directories(nocturne PRIVATE ${tensorflow_include} "src/" $ENV{HOME}/Documents/tflite_build/flatbuffers/include)
target_link_libraries(nocturne PRIVATE ${libtf})
