cmake_minimum_required(VERSION 3.28)
project(SimpleDB)

set(CMAKE_CXX_STANDARD 23)

add_executable(SimpleDB main.cpp)

find_package(argparse REQUIRED)
find_package(fmtlog REQUIRED)
find_package(magic_enum REQUIRED)

add_subdirectory(include)

add_subdirectory(tests)