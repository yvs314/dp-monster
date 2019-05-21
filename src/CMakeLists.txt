cmake_minimum_required(VERSION 3.9) #bumped 3.0. to 3.9 to make use of find_package(OpenMP REQUIRED)

project(dpm CXX) #CXX explicitly makes it a C++ project

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dpm monster-main.cpp
                                                    bitset-base.cpp
                                                    prec-ord.cpp
                                                    reader-base.cpp
                                                    dp-recovery.cpp)

find_package(OpenMP REQUIRED) # Find the OpenMP package
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES}) # Link against it for C++

install(TARGETS dpm RUNTIME DESTINATION bin)
