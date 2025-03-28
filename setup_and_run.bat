@echo off
setlocal enabledelayedexpansion

:: 1. Activate virtual environment
if exist venv\Scripts\activate.bat (
    call venv\Scripts\activate
) else (
    echo Virtual environment not found. Please create it first using 'python -m venv venv'
    exit /b 1
)

:: 2. Clean and build the project
if exist build rmdir /s /q build
mkdir build
cd build

:: Dynamically find Python executable
for /f "delims=" %%a in ('python -c "import sys; print(sys.executable)"') do set "PYTHON_EXEC=%%a"

:: Run cmake with the correct Python executable
cmake .. -DPython3_EXECUTABLE="%PYTHON_EXEC%"
if %errorlevel% neq 0 (
    echo CMake configuration failed
    exit /b 1
)

:: Run cmake build
cmake --build .
if %errorlevel% neq 0 (
    echo Project build failed
    exit /b 1
)

:: 3. Run your Python script
cd ..\python_gui
:: CHANGE THIS TO main_app.py WHEN FINISHED
python main_app.py