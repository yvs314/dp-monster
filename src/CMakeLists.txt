cmake_minimum_required(VERSION 3.0)

project(dpm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(dpm monster-main.cpp
                                                    bitset-base.cpp
                                                    prec-ord.cpp
                                                    reader-base.cpp
                                                    dp-recovery.cpp)

install(TARGETS dpm RUNTIME DESTINATION bin)
