cmake_minimum_required(VERSION 3.10)

# 项目名称
project(RISCV_CPU_Simulator CXX)

# 设置 C++ 标准（通常 C++11 或 C++17 足够）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动收集 src 目录下的所有 .cpp 源文件
file(GLOB SOURCES "src/*.cpp")

# 包含头文件目录（让 #include "ALU.h" 能直接被找到）
include_directories(src)

# 将可执行文件输出到根目录下，而不是 build 文件夹内
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 生成可执行文件 code
add_executable(code src/simulator.cpp)
target_compile_options(code PRIVATE -Ofast)