cmake_minimum_required(VERSION 3.0.0)

# Some compiler flags
set(CMAKE_CXX_STANDARD 11)
if (MSVC) # Multi core in VS
    SET(PUBLIC CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "/MP")
endif()

project(Games)
add_definitions(-DONUT_USE_SDL)

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Onut
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/onut onut)
list(APPEND libs PUBLIC libonut)
list(APPEND includes PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/onut/include/)

# Common
list(APPEND includes PUBLIC ${CMAKE_SOURCE_DIR}/common/)

add_subdirectory(./games/freecell)
