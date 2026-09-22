cmake_minimum_required(VERSION 3.18)
project(native_brain LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(brain_core STATIC
    core/brain_core.cpp
)

target_include_directories(brain_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ONA and the minimal AtomSpace implementation will be attached here
# after their source dependency closures are verified.
