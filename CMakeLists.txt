# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(RISCVSim VERSION 1.0 LANGUAGES CXX)

# Set C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files (you can list them individually or use GLOB)
set(SOURCE_FILES
    ../main.cpp
    ../src/instruction.cpp
    ../src/lexer.cpp
    ../src/pipeline.cpp
)

# Include directories for headers
include_directories(include)

# Create an executable from source files
add_executable(riscv-sim ${SOURCE_FILES})
