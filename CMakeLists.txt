cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PChorAnalyzer)

# Adds warnings and allows debug with gdb
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-Wall -Wextra -O2)

include_directories(
    ./src/utils
    ./src/pchor
    ./src/analyzer
)

add_executable(pchoranalyzer
    ./src/main.cpp
    ./src/utils/Utils.hpp ./src/utils/Utils.cpp
    ./src/pchor/pchor.hpp ./src/pchor/pchor.cpp
    ./src/analyzer/AstVisitor.hpp ./src/analyzer/AstVisitor.cpp
    ./src/analyzer/Analyzer.hpp ./src/analyzer/Analyzer.cpp
)

cmake_policy(SET CMP0167 NEW)
