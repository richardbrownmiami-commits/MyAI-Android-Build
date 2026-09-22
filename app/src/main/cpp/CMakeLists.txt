cmake_minimum_required(VERSION 3.22.1)
project(myai_brain_android LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BRAIN_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../brain)
set(ONA_ROOT ${BRAIN_ROOT}/nars/ona)
set(ONA_RULE_TABLE ${ONA_ROOT}/src/RuleTable.c)

if(NOT EXISTS "${ONA_RULE_TABLE}")
    message(FATAL_ERROR "ONA RuleTable.c is missing. Run brain/nars/generate_rule_table.sh before the Android build.")
endif()

set(ONA_SOURCES
    ${ONA_ROOT}/src/Cycle.c
    ${ONA_ROOT}/src/Decision.c
    ${ONA_ROOT}/src/Event.c
    ${ONA_ROOT}/src/Globals.c
    ${ONA_ROOT}/src/HashTable.c
    ${ONA_ROOT}/src/Inference.c
    ${ONA_ROOT}/src/InvertedAtomIndex.c
    ${ONA_ROOT}/src/Memory.c
    ${ONA_ROOT}/src/NAL.c
    ${ONA_ROOT}/src/NAR.c
    ${ONA_ROOT}/src/Narsese.c
    ${ONA_ROOT}/src/OccurrenceTimeIndex.c
    ${ONA_ROOT}/src/PriorityQueue.c
    ${ONA_ROOT}/src/Stack.c
    ${ONA_ROOT}/src/Stamp.c
    ${ONA_ROOT}/src/Stats.c
    ${ONA_ROOT}/src/Table.c
    ${ONA_ROOT}/src/Term.c
    ${ONA_ROOT}/src/Truth.c
    ${ONA_ROOT}/src/Usage.c
    ${ONA_ROOT}/src/Variable.c
    ${ONA_RULE_TABLE}
)

add_library(myai_brain SHARED
    brain_jni.cpp
    ${BRAIN_ROOT}/atomspace/core/atom.cpp
    ${BRAIN_ROOT}/atomspace/core/atomspace.cpp
    ${BRAIN_ROOT}/representation/core/bridge.cpp
    ${BRAIN_ROOT}/representation/core/cr.cpp
    ${BRAIN_ROOT}/brain/brain_orchestrator.cpp
    ${BRAIN_ROOT}/nars/core/ona_bridge.cpp
    ${BRAIN_ROOT}/core/brain_core.cpp
    ${ONA_SOURCES}
)

target_include_directories(myai_brain PRIVATE
    ${BRAIN_ROOT}/include
    ${BRAIN_ROOT}/atomspace/include
    ${BRAIN_ROOT}/representation/include
    ${BRAIN_ROOT}/nars/include
    ${ONA_ROOT}/src
)

target_compile_definitions(myai_brain PRIVATE _POSIX_C_SOURCE=199506L)
target_compile_options(myai_brain PRIVATE -Wno-unused-parameter -Wno-unused-variable -Wno-dollar-in-identifier-extension)

find_library(log-lib log)
find_library(m-lib m)
find_library(android-lib android)
target_link_libraries(myai_brain PRIVATE ${log-lib} ${m-lib} ${android-lib})
