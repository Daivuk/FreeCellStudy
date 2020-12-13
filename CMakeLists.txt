cmake_minimum_required(VERSION 3.0.0)

project(Games)

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Onut
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/onut onut)
list(APPEND libs PUBLIC libonut)
list(APPEND includes PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/onut/include/)

# Common
list(APPEND includes PUBLIC ${CMAKE_SOURCE_DIR}/common/)

add_subdirectory(./games/freecell)
