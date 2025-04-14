cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PChorAnalyzer)

# Find Clang and LLVM
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Adds warnings and allows debug with gdb
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-Wall -Wextra -O2)


# Setup for Lexer and Semantics Parser of Language, which generates PchorAST
add_library(PchorCore STATIC
    ./src/pchor/PchorAST.cpp
    ./src/pchor/PchorFileWrapper.hpp
    ./src/pchor/PchorParser.cpp
    ./src/pchor/PchorTokenizer.cpp
)

target_include_directories(PchorCore PRIVATE
    ./src/pchor
    ./src/utils
)


# Plugin is shared library which is linked to PchorCore later
add_library(PchorAnalyzerPlugin SHARED
    ./src/analyzer/AstVisitor.cpp
    ./src/analyzer/Analyzer.cpp
    ./src/utils/Utils.cpp
    ./src/analyzer/Plugin.cpp
)



# Include directories for plugin
target_include_directories(PchorAnalyzerPlugin SYSTEM PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    /usr/lib/llvm-18/include
    ./src/pchor
    ./src/utils
    ./src/analyzer
)

# Link against Clang libraries
target_link_libraries(PchorAnalyzerPlugin
    PchorCore
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangTooling
    ${LLVM_LIBRARIES}
)

# Set the plugin name
set_target_properties(PchorAnalyzerPlugin PROPERTIES
    OUTPUT_NAME "PchorAnalyzerPlugin"
)

cmake_policy(SET CMP0167 NEW)