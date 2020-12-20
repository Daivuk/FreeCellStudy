cmake_minimum_required(VERSION 3.0.0)

# Project name
project(CFreeCell)

# Remove secure warnings on functions like sprintf -> s_sprintf
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Project files
file(GLOB src_files ./src/*.*)

# Executable
# add_executable(${PROJECT_NAME} WIN32 ${src_files})
add_executable(${PROJECT_NAME} ${src_files})

# SDL
set(HAVE_LIBC ON)
add_subdirectory(./SDL/)

# Work dir
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")

# Include paths
target_include_directories(${PROJECT_NAME} PUBLIC ./SDL/include/ ./src/)

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2-static)
