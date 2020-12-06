cmake_minimum_required(VERSION 3.0.0)

project(AdventOfCode)

# Onut
add_subdirectory(./thirdparty/onut onut)
list(APPEND libs PUBLIC libonut)
list(APPEND includes PUBLIC ./thirdparty/onut/include/)

add_subdirectory(./1)
add_subdirectory(./2)
add_subdirectory(./3)
add_subdirectory(./4)
add_subdirectory(./5)
add_subdirectory(./6)
