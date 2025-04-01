#!/bin/bash

# Exit on any error
set -e

# 1. Activate virtual environment
if [ -f venv/bin/activate ]; then
    source venv/bin/activate
else
    echo "Virtual environment not found. Please create it first using 'python3.13 -m venv venv'"
    exit 1
fi

# 2. Clean and build the project
rm -rf build
mkdir -p build && cd build

# Get the Python executable from the activated venv
PYTHON_EXEC=$(which python)

# Get pybind11 CMake directory
PYBIND11_CMAKE_DIR=$(python -c "import pybind11; print(pybind11.get_cmake_dir())")

# Run cmake with the correct Python executable and pybind11_DIR
cmake .. -DPython3_EXECUTABLE="$PYTHON_EXEC" -Dpybind11_DIR="$PYBIND11_CMAKE_DIR"

# Build the project
make

# 3. Set PYTHONPATH to include the build directory
export PYTHONPATH="/Users/samholtzman/Documents/GitHub/Abalone/build:$PYTHONPATH"

# 4. Run your Python script
cd ..  # Back to Abalone/
python -m python_gui.main_app