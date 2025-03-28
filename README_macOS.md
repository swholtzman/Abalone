# Abalone AI Setup on macOS

This guide will help you set up the project on macOS, including installing Python 3.13, G++, CMake, PyQt5, and pybind11, and setting up the development environment.

## 1. Install Python 3.13

### Download and Install Python 3.13
1. Download **Python 3.13** from the [official Python website](https://www.python.org/downloads/release/python-3130/).
2. Run the installer, and **ensure that "Add Python to PATH"** is checked during installation.
3. Verify the installation:

   ```bash
   python3.13 --version
   ```

   You should see something like:
   ```
   Python 3.13.x
   ```

## 2. Install G++ (MinGW or Xcode Command Line Tools)

### Option 1: Install Xcode Command Line Tools (easiest for macOS)
Open Terminal and run:

```bash
xcode-select --install
```

This will install gcc (which includes g++).

Verify installation:

```bash
g++ --version
```

You should see a version of G++ installed by Xcode (e.g., Apple clang version).

### Option 2: Install Homebrew (Alternative Method)
If you want to use a more recent GCC:

1. Install Homebrew (if you haven't already):

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. Install GCC:

   ```bash
   brew install gcc
   ```

3. Verify g++ installation:

   ```bash
   g++ --version
   ```

## 3. Install CMake
Install CMake using Homebrew:

```bash
brew install cmake
```

Verify installation:

```bash
cmake --version
```

## 4. Set Up Python Virtual Environment and Dependencies

### 1. Create a Virtual Environment
Create a virtual environment:

```bash
python3.13 -m venv venv
```

Activate the virtual environment:

```bash
source venv/bin/activate
```

### 2. Install Dependencies
Install required Python packages:

```bash
pip install PyQt5 pybind11 cmake
```

## 5. Setting Up the Project

### 1. Make Setup Script Executable
Before running the setup script, you need to make it executable:

```bash
chmod +x setup_and_run.sh
```

### 2. Build the Project
Run the setup script to clean and build the project:

```bash
./setup_and_run.sh
```

The script will:
- Clean the build/ directory
- Rebuild the project using Python 3.13
- Run the Python script

### 3. Running the Project Manually (if needed)
If you want to run it manually:

Navigate to the python_gui/ directory:

```bash
cd ../python_gui
```

Run the Python script:

```bash
python3.13 main_app.py
```

## Troubleshooting
- **Python not found**: Ensure you have Python 3.13 installed and correctly added to PATH.
- **G++ issues**: If g++ is not found, try using `xcode-select --install` or install gcc via Homebrew.
- **PyQt5 installation**: If you encounter any issues, try:
  ```bash
  pip install --upgrade pip
  pip install PyQt5
  ```
- **Setup script permissions**: Make sure the setup script is executable:
  ```bash
  chmod +x setup_and_run.sh
  ```
- **pybind11 installation**: If you have issues installing pybind11, try:
  ```bash
  pip install --upgrade pip
  pip install pybind11 cmake
  ```

Let me know if you need any help!