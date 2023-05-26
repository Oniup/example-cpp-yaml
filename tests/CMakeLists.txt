cmake_minimum_required(VERSION 3.2)

project(yaml_unit_tests)

add_executable(${CMAKE_PROJECT_NAME} main.cpp ../yaml.hpp ../yaml.cpp)
