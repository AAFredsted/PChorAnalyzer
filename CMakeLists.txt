cmake_minimum_required(VERSION 3.20)

# Set compiler and standard
# set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project name
project(PChorAnalyzer)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Add build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Add PchorCore library
add_library(PchorCore SHARED
    ./src/pchor/ast/PchorAST.cpp
    ./src/pchor/ast/PchorProjection.cpp
    ./src/pchor/parser/PchorParser.cpp
    ./src/pchor/parser/PchorTokenizer.cpp
)

target_compile_options(PchorCore PRIVATE
    -isystem /usr/lib/llvm-18/include
)
target_include_directories(PchorCore PRIVATE
    ./src/pchor
    ./src/utils)

target_compile_options(PchorCore PRIVATE -Wall -Wextra -O2)

# Add PchorAnalyzerPlugin library
add_library(PchorAnalyzerPlugin SHARED
    ./src/analyzer/visitors/AstVisitor.cpp
    ./src/analyzer/visitors/CASTValidator.cpp
    ./src/analyzer/utils/CASTAnalyzerUtils.cpp
    ./src/analyzer/utils/ContextManager.cpp
    ./src/utils/Utils.cpp
    ./src/analyzer/Plugin.cpp
)


target_compile_options(PchorAnalyzerPlugin PRIVATE
    -isystem /usr/lib/llvm-18/include
)
target_include_directories(PchorAnalyzerPlugin SYSTEM PRIVATE
    ./src/pchor
    ./src/utils
    ./src/analyzer
)

target_compile_options(PchorAnalyzerPlugin PRIVATE -Wall -Wextra -O2)

# Link against Clang libraries
execute_process(COMMAND llvm-config-18 --libdir OUTPUT_VARIABLE LLVM_LIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
link_directories(${LLVM_LIB_DIR})

#Include 
link_directories(/usr/lib/llvm-18/lib)

target_link_libraries(PchorAnalyzerPlugin
    PchorCore
    /usr/lib/llvm-18/lib/libclangAST.a
    /usr/lib/llvm-18/lib/libclangASTMatchers.a
    /usr/lib/llvm-18/lib/libclangBasic.a
    /usr/lib/llvm-18/lib/libclangFrontend.a
    /usr/lib/llvm-18/lib/libclangTooling.a
)

# Installation rules
install(TARGETS PchorCore PchorAnalyzerPlugin
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ./src/pchor DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ./src/analyzer DESTINATION include FILES_MATCHING PATTERN "*.hpp")