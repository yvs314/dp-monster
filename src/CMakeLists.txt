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
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES}) # Link against it for C++

install(TARGETS dpm RUNTIME DESTINATION bin)
