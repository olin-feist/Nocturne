project(Nocturne)
cmake_minimum_required(VERSION 3.22)

set(cross_path $ENV{HOME}/.local/cross)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_C_COMPILER ${cross_path}/bin/gcc-arm)
set (CMAKE_CXX_COMPILER ${cross_path}/bin/g++-arm)
set (CMAKE_LINKER ${cross_path}/bin/ld-arm)
set (CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set( CMAKE_INSTALL_RPATH "/usr/local/lib")

set(CMAKE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(rknn_path "$ENV{HOME}/Documents/RK_NPU_SDK_1.5.0/rknpu2/runtime/RK356X/Linux/librknn_api")

#lib files
set(librknn "${rknn_path}/aarch64/librknnrt.so")
#set(libstd++ "${cross_path}/aarch64-rockchip-linux/lib64/libstdc++.so.6")
#set(libglibc "${cross_path}/aarch64-rockchip-linux/lib/libc.so")


#include paths
set(rknn_include "${rknn_path}/include/")
set(std_include "${cross_path}/aarch64-rockchip-linux/include/")

#add_compile_options(-static)

add_executable(nocturne src/test.cpp)
set_target_properties( nocturne PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
) 

target_include_directories(nocturne PRIVATE ${rknn_include} "src/")
target_link_libraries(nocturne PRIVATE ${librknn})
