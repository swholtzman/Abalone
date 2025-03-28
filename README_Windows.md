# Abalone AI Setup on Windows

This guide will help you set up the project on Windows, including installing Python 3.13, G++ (via MinGW or MSYS2), CMake, PyQt5, and pybind11, and setting up the development environment.

## 1. Install Python 3.13

### Download and Install Python 3.13
1. Download **Python 3.13** from the [official Python website](https://www.python.org/downloads/release/python-3130/).
2. Run the installer, and **ensure that "Add Python to PATH"** is checked during installation.
3. Verify the installation:

   ```cmd
   python --version
   ```

   You should see something like:
   ```
   Python 3.13.x
   ```

## 2. Install G++ (MinGW or MSYS2)

### Option 1: Install MinGW (GNU Compiler Collection)
1. Download MinGW-w64 from SourceForge.
2. Install MinGW, ensuring you select the correct architecture (x86_64) and threads (posix).
3. Add MinGW to your PATH:
   - Right-click This PC → Properties → Advanced System Settings → Environment Variables
   - Add the bin folder of MinGW to the PATH (e.g., `C:\Program Files\mingw-w64\...\bin`)

Verify G++ installation:

```cmd
g++ --version
```

You should see a version like:
```
g++ (MinGW.org GCC-6.3.0-1) 6.3.0
```

### Option 2: Install MSYS2 (Alternative to MinGW)
1. Download MSYS2 from MSYS2.org.
2. Open the MSYS2 terminal and run:

   ```bash
   pacman -Syu
   pacman -S base-devel mingw-w64-x86_64-toolchain
   ```

3. Install G++ via MSYS2:

   ```bash
   pacman -S mingw-w64-x86_64-gcc
   ```

4. Add MSYS2 to your PATH by copying the bin path (e.g., `C:\msys64\mingw64\bin`).

Verify G++ installation:

```bash
g++ --version
```

## 3. Install CMake
1. Download and Install CMake from CMake.org.

Verify installation:

```cmd
cmake --version
```

## 4. Set Up Python Virtual Environment and Dependencies

### 1. Create a Virtual Environment
Create a virtual environment:

```cmd
python -m venv venv
```

Activate the virtual environment:

```cmd
venv\Scripts\activate
```

### 2. Install Dependencies
Install required Python packages:

```cmd
pip install PyQt5 pybind11 cmake
```

## 5. Setting Up the Project

### 1. Build the Project
Run the setup script to clean and build the project:

```cmd
.\setup_and_run.bat
```

The script will:
- Clean the build/ directory
- Rebuild the project using Python 3.13
- Run the Python script

### 2. Running the Project Manually (if needed)
If you want to run it manually:

Navigate to the python_gui/ directory:

```cmd
cd ..\python_gui
```

Run the Python script:

```cmd
python main_app.py
```

## Troubleshooting
- **Python not found**: Ensure you have Python 3.13 installed and correctly added to PATH.
- **G++ issues**: If g++ is not found, try installing via MinGW or MSYS2.
- **PyQt5 installation**: If you encounter any issues, try:
  ```cmd
  pip install --upgrade pip
  pip install PyQt5
  ```
- **pybind11 installation**: If you have issues installing pybind11, try:
  ```cmd
  pip install --upgrade pip
  pip install pybind11 cmake
  ```
- **Setup script permissions (macOS/Linux)**: If you're on macOS or Linux, you may need to make the setup script executable by running:
  ```bash
  chmod +x setup_and_run.sh
  ```

Let me know if you need any help!