cmake_minimum_required(VERSION 3.10)
project(EDD_Proyecto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project structure
set(SRC_DIR src)
set(BIN_DIR bin)

# Source files
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/csvReader/csvReader.cpp
    ${SRC_DIR}/mafiaFamily/mafiaTree.cpp
)

# Include directories
include_directories(${SRC_DIR})
include_directories(${SRC_DIR}/external)

# Executable
add_executable(mafia_system ${SOURCES})

# Output directory for the executable
set_target_properties(mafia_system PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BIN_DIR}")
