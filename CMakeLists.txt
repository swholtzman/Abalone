cmake_minimum_required(VERSION 3.14)
project(abalone_ai)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Properly find the correct Python version via Python3 module
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

# Define the pybind11 module
pybind11_add_module(abalone_ai
    cpp_backend/AbaloneAI.cpp
    cpp_backend/Board.cpp
    cpp_backend/TranspositionTable.cpp
    cpp_backend/AbaloneAiPybindWrapper.cpp
)

# Explicitly link against Python3 (from Python3_EXECUTABLE)
target_link_libraries(abalone_ai PRIVATE Python3::Python)