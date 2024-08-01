cmake_minimum_required (VERSION 3.14)
project (GRE)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(FetchContent)
include(ExternalProject)

set(TBB_DIR "~/intel/oneapi/tbb/latest/lib/cmake/tbb")

find_package(Threads REQUIRED)
link_libraries(Threads::Threads)
include_directories(
        ~/intel/oneapi/mkl/latest/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/parlaylib/include/
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(third_party/gflags)

# find_package(MKL REQUIRED)
find_package(JeMalloc REQUIRED)
find_package(TBB REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
        -DNoHelp  # for verlib
        -g
        -O3 -march=native
        -Wall -Wextra
        -fno-omit-frame-pointer -ggdb -mbmi -mlzcnt
        -mavx -mavx2 -mavx512f
)

include_directories(${MKL_INCLUDE_DIRS})
include_directories(${JEMALLOC_INCLUDE_DIR})
include_directories(SYSTEM src/competitor/hot/src/libs/hot/commons/include)
include_directories(SYSTEM src/competitor/hot/src/libs/hot/rowex/include)
include_directories(SYSTEM src/competitor/hot/src/libs/hot/single-threaded/include)
include_directories(SYSTEM src/competitor/hot/src/libs/idx/content-helpers/include)

# add_subdirectory(src/competitor/wormhole)
# add_subdirectory(src/competitor/wormhole_u64)
# add_subdirectory(src/competitor/masstree)

# microbench
add_executable(microbench
        ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark/microbench.cpp
    )

target_link_libraries(microbench
        PRIVATE
        # WORMHOLE
        # WORMHOLEU64
        # MASSTREE
)

target_link_libraries(microbench PUBLIC ${JEMALLOC_LIBRARIES} ${MKL_LIBRARIES} TBB::tbb)


add_executable(main
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ours/main.cpp
    )
target_link_libraries(main
        PRIVATE
        # WORMHOLE
        # WORMHOLEU64
        # MASSTREE
)
target_link_libraries(main PUBLIC
        gflags::gflags
        ${JEMALLOC_LIBRARIES} ${MKL_LIBRARIES} TBB::tbb
)
