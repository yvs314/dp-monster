cmake_minimum_required(VERSION 3.0)

project(hellokdevelop)

#add_library(BITSETBASE_H_ STATIC bitset-base.cpp bitset-base.h)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#add_executable(hellokdevelop main.cpp)
add_executable(a monster-main.cpp
                                                    bitset-base.cpp
                                                    prec-ord.cpp
                                                    reader-base.cpp
                                                    dp-recovery.cpp)

install(TARGETS a RUNTIME DESTINATION bin)

