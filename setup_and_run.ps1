# setup_and_run_mingw.ps1
$ErrorActionPreference = "Stop"

# 1. Delete existing virtual environment if it exists
if (Test-Path "venv") {
    Write-Host "Deleting existing virtual environment..."
    Remove-Item -Recurse -Force "venv"
}

# 2. Create a new virtual environment using python3.12
Write-Host "Creating new virtual environment..."
python -m venv venv

# Activate the virtual environment
. .\venv\Scripts\Activate.ps1

# Ensure the venv has a python3.12 command by copying python.exe
# Copy-Item -Path .\venv\Scripts\python.exe -Destination .\venv\Scripts\python.exe -Force

# 3. Upgrade pip and install dependencies
Write-Host "Installing dependencies: pyqt5, pillow, pybind11, and cmake..."
python -m pip install --upgrade pip
python -m pip install pyqt5 pillow pybind11 cmake nuitka

# Retrieve the pybind11 CMake directory from the installed package
$pybind11CmakeDir = & python -m pybind11 --cmakedir
Write-Host "Using pybind11 CMake directory: $pybind11CmakeDir"

# 4. Clean and build the project
if (Test-Path "build") {
    Write-Host "Cleaning previous build..."
    Remove-Item -Recurse -Force "build"
}
Write-Host "Creating build directory..."
New-Item -ItemType Directory -Path build | Out-Null
Push-Location build

# Dynamically find the Python executable from the venv
$pythonExec = (Get-Command python).Source
Write-Host "Using Python executable: $pythonExec"

# Run CMake with the MinGW Makefiles generator and explicit compiler paths.
cmake .. -G "MinGW Makefiles" `
    -DPython3_EXECUTABLE="$pythonExec" `
    -Dpybind11_DIR="$pybind11CmakeDir" `
    -DCMAKE_MAKE_PROGRAM="C:\ProgramData\mingw64\mingw64\bin\mingw32-make.exe" `
    -DCMAKE_C_COMPILER="C:\ProgramData\mingw64\mingw64\bin\x86_64-w64-mingw32-gcc.exe" `
    -DCMAKE_CXX_COMPILER="C:\ProgramData\mingw64\mingw64\bin\x86_64-w64-mingw32-g++.exe"

Write-Host "Building the project..."
& "C:\ProgramData\mingw64\mingw64\bin\mingw32-make.exe"
Pop-Location

# 4.5. Copy the built module from the build directory to the python_gui folder
Write-Host "Copying built module to python_gui folder..."
Copy-Item -Path "build\abalone_ai.cp312-win_amd64.pyd" -Destination "python_gui\abalone_ai.cp312-win_amd64.pyd" -Force

# 4.6. Copy necessary runtime DLLs to the python_gui folder
$mingwBin = "C:\ProgramData\mingw64\mingw64\bin"
Write-Host "Copying MinGW runtime DLLs from $mingwBin to python_gui folder..."
Copy-Item -Path "$mingwBin\libstdc++-6.dll" -Destination "python_gui\" -Force
Copy-Item -Path "$mingwBin\libgcc_s_seh-1.dll" -Destination "python_gui\" -Force
Copy-Item -Path "$mingwBin\libwinpthread-1.dll" -Destination "python_gui\" -Force

# 4.7. Copy Python 3.12 DLL to the python_gui folder
# Determine the Python DLL path from the python executable location.
$pythonDir = Split-Path $pythonExec
$pythonDllPath = Join-Path $pythonDir "python.dll"
if (Test-Path $pythonDllPath) {
    Write-Host "Copying Python DLL from $pythonDllPath to python_gui folder..."
    Copy-Item -Path $pythonDllPath -Destination "python_gui\" -Force
} else {
    Write-Host "WARNING: Python DLL not found at $pythonDllPath. Please update the path if needed."
}

# 5. Run the Python GUI for testing
# Also add the MinGW runtime DLL directory to the PATH as an extra precaution.
$env:Path = "$mingwBin;" + $env:Path
Write-Host "Running the Python GUI..."
Push-Location python_gui
python main_app.py
Pop-Location
