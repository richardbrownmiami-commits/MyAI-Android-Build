cmake_minimum_required(VERSION 3.22.1)
project(myai_brain_android LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BRAIN_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../brain)
add_library(myai_brain SHARED
    brain_jni.cpp
    ${BRAIN_ROOT}/atomspace/core/atom.cpp
    ${BRAIN_ROOT}/atomspace/core/atomspace.cpp
    ${BRAIN_ROOT}/representation/core/bridge.cpp
    ${BRAIN_ROOT}/core/brain_core.cpp
)
target_include_directories(myai_brain PRIVATE
    ${BRAIN_ROOT}/include
    ${BRAIN_ROOT}/atomspace/include
    ${BRAIN_ROOT}/representation/include
)
find_library(log-lib log)
target_link_libraries(myai_brain PRIVATE ${log-lib})
