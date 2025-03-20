cmake_minimum_required (VERSION 3.15)
project (Transformation)

find_package(Eigen3 REQUIRED)
include_directories(D:/eigen-3.4.0)

add_executable (Transformation main.cpp)
