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

# Dynamically find Python executable
PYTHON_EXEC=$(which python3.13)

# Run cmake with the correct Python executable
cmake .. -DPython3_EXECUTABLE="$PYTHON_EXEC"

# Build the project
make

# 3. Run your Python script
cd ../python_gui
# CHANGE THIS TO main_app.py WHEN FINISHED
python3.13 main_app.py