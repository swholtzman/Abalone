#!/bin/bash

# Exit on any error
set -e

# 1. Delete existing virtual environment if it exists
if [ -d "venv" ]; then
    echo "Deleting existing virtual environment..."
    rm -rf venv
fi

# 2. Create a new virtual environment
echo "Creating new virtual environment..."
python3.13 -m venv venv

# Activate the virtual environment
source venv/bin/activate

# 3. Upgrade pip and install dependencies
echo "Installing dependencies: pyqt5, pillow, pybind11, and cmake..."
pip install --upgrade pip
pip install pyqt5 pillow pybind11 cmake

# 4. Clean and build the project
rm -rf build
mkdir -p build && cd build

# Dynamically find the Python executable
PYTHON_EXEC=$(which python3.13)

# Run cmake with the correct Python executable
cmake .. -DPython3_EXECUTABLE="$PYTHON_EXEC"

# Build the project
make

# 5. Run your Python script
cd ../python_gui
# CHANGE THIS TO main_app.py WHEN FINISHED
python3.13 main_app.py