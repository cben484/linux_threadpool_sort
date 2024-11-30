cmake_minimum_required(VERSION 3.25)
project( THREADPOOL LANGUAGES CXX VERSION 0.1 )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_path(INCLUDE_DIR ThreadPool.h HINTS ${CMAKE_SOURCE_DIR}/include)

add_library(
        solver 
        SHARED   
        Merge.cpp 
        Solver.cpp
    )
target_include_directories(solver PUBLIC ${INCLUDE_DIR})

add_executable(main main.cpp)
target_include_directories(main PUBLIC ${INCLUDE_DIR})
target_link_libraries(main PUBLIC solver)