# Cross Compiler config
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

set (CMAKE_SYSROOT /)
set (cross_path $ENV{HOME}/.local/cross)

set (CMAKE_LINKER ${cross_path}/bin/aarch64-rockchip-linux-ld)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_C_COMPILER ${cross_path}/bin/aarch64-rockchip-linux-gcc)
set (CMAKE_CXX_COMPILER ${cross_path}/bin/aarch64-rockchip-linux-g++)

set (CMAKE_FIND_ROOT_PATH  ${cross_path}/aarch64-rockchip-linux)
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#Project config
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

project(Nocturne VERSION 1.0) 
cmake_minimum_required(VERSION 3.22)

set(CMAKE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# TF Lite
set(TENSORFLOW_SOURCE_DIR "" CACHE PATH
  "Directory that contains the TensorFlow project"
)
if(NOT TENSORFLOW_SOURCE_DIR)
  get_filename_component(TENSORFLOW_SOURCE_DIR
    "$ENV{HOME}/Documents/tensorflow_src"
    ABSOLUTE
  )
endif()
add_subdirectory(
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
  EXCLUDE_FROM_ALL
)

# Open CL
set(OpenCL_INCLUDE_DIRS $ENV{HOME}/Documents/mesa/include)

# Open CV
set(OpenCV_DIR $ENV{HOME}/Documents/opencv_build)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs dnn)

# Exe Config
add_executable(nocturne src/nocturne.cpp src/capture.cpp src/utils.cpp src/sensor.cpp src/display.cpp src/detect.cpp)
set_target_properties( nocturne PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
) 

target_include_directories(nocturne PRIVATE "src/" ${OpenCV_INCLUDE_DIRS} ${OpenCL_INCLUDE_DIRS})
target_link_libraries(nocturne PRIVATE tensorflow-lite ${OpenCV_LIBS} OpenCL)
