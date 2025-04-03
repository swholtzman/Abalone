# Abalone Game

An interactive implementation of the **Abalone board game** with an AI opponent designed using heuristic-based strategies. Built using a hybrid of **C++** for performance-critical simulation and **Python (PyQt5)** for a sleek graphical user interface.

---

## 🧠 Introduction

This project blends the strategic gameplay of Abalone with artificial intelligence to explore optimized gameplay through heuristic search and decision-making algorithms. It's a cross-platform solution aimed at showcasing AI implementation in board games using modern tools like `pybind11`, `CMake`, and `Nuitka`.

---

## 📚 Table of Contents

- [Features](#-features)
- [Requirements](#-requirements)
- [Repository Structure](#-repository-structure)
- [Installation & Setup](#-installation--setup)
  - [macOS](#macos)
  - [Windows](#windows)
- [Usage](#-usage)
  - [Running the Game](#running-the-game)
  - [Simulating a Game](#simulating-a-game-c-simulation)
- [Packaging (Windows)](#-packaging-windows)
- [AI Strategy Enhancements](#-ai-strategy-enhancements)
- [Troubleshooting](#-troubleshooting)
- [Contributors](#-contributors)
- [License](#-license)

---

## 🌟 Features

- C++ backend for efficient simulation and AI logic
- PyQt5-based graphical interface
- Cross-platform support (macOS and Windows)
- Heuristic-based AI bot with transposition tables
- Easy-to-use build scripts
- Standalone Windows packaging via Nuitka

---

## ⚙️ Requirements

### C++ Compiler:
- **macOS**: `clang++` or `g++`
- **Windows**: `MinGW` (preferred) or MSVC

### Python:
- **macOS**: Python 3.13
- **Windows**: Python 3.12

### Tools:
- `CMake`
- `Nuitka` (for Windows packaging)
- `pybind11`
- `PyQt5`
- `Pillow`

> 💡 **Tip**: On Windows, use [Chocolatey](https://chocolatey.org/) for installing dependencies.

---

## 📁 Repository Structure

```plaintext
/abalone-game/
│
├── cpp_backend/             # C++ source code: game logic, AI, simulation
├── python_gui/              # Python GUI (PyQt5)
├── public/resources/        # Resource files (images, fonts, etc.)
├── old/                     # Legacy or deprecated content
├── revisionWorktree/        # AI enhancements: mobility & positional logic
│
├── setup_and_run.sh         # macOS setup & run script
├── setup_and_run.ps1        # Windows setup script
├── install_dependencies.ps1 # Script for installing Windows tools
├── build_with_nuitka.ps1    # Windows Nuitka packaging script
├── build_with_nuitka.sh     # Experimental onefile/pybind build (Linux/macOS)
├── .editorconfig            # Code formatting configuration
├── CMakeLists.txt           # CMake configuration
└── README.md
```

---

## 🛠️ Installation & Setup

### macOS

1. **Install Dependencies**  
   Make sure you have:
   - Python 3.12
   - `clang++` or `g++`
   - `CMake`

2. **Run Setup Script**
   ```bash
   ./setup_and_run.sh
   ```

   This will:
   - Create a fresh virtual environment
   - Install Python dependencies
   - Build the C++ backend
   - Launch the GUI

---

### Windows

1. **Install Dependencies**
   Run the following in **elevated PowerShell**:
   ```powershell
   .\install_dependencies.ps1
   ```

   Or manually via Chocolatey:
   ```powershell
   choco install python --version=3.12.0 -y
   choco install mingw -y
   choco install cmake -y
   ```

2. **Run Setup Script**
   ```powershell
   .\setup_and_run.ps1
   ```

   This will:
   - Set up virtual environment
   - Install Python packages
   - Compile C++ with MinGW
   - Copy required DLLs and modules
   - Launch the GUI

---

## ▶️ Usage

### Running the Game

- **macOS**: After `setup_and_run.sh`, the GUI starts automatically.
- **Windows**: After `setup_and_run.ps1`, the GUI starts. If using Nuitka, launch from the distribution folder.

---

### Simulating a Game (C++ Simulation)

1. Navigate to `cpp_backend/`

2. **Build the simulation:**

Using `make`:
```bash
make all
```
or
```bash
make play_game
```

Without Make:
```bash
g++ -std=c++17 play_game.cpp Board.cpp TranspositionTable.cpp AbaloneAI.cpp -o play_game
```

3. **Run the Simulation:**
```bash
./play_game
```

> Output files like `moves_made.txt` and `visualize_output.txt` will be generated.

---

## 📦 Packaging (Windows)

To create a distributable version using Nuitka:

```powershell
.build_with_nuitka.ps1
```

This script:
- Compiles the Python frontend
- Bundles resources from `public/`
- Creates a folder-based distribution

> 🔁 Ensure your game reads resources relative to the distribution structure (e.g., `../public`).

---

## 🔬 AI Strategy Enhancements

Additional research and strategy work are maintained in the `revisionWorktree/` directory. It includes:
- Refined AI logic with mobility and positional awareness
- Experimental improvements not yet merged into main `cpp_backend`

---

## 🧩 Troubleshooting

- **Missing DLLs on Windows?** Make sure MinGW and Python DLLs are copied to the GUI folder.
- **GUI not launching?** Check that PyQt5 and other packages are installed in your virtual environment.
- **Python version mismatch?** Windows users should use **Python 3.12**, not 3.13, to avoid MinGW issues.

---

## 👥 Contributors

- **AIML Group 1**
- PRs and feedback welcome!

---

## Patch Notes
### Patch 1.2.1
- Pushed April 3, 2025 12:33am ; Sam Holtzman
- Highlight suggested moves based off agent "Next Move"
  - Only highlights the suggested path for tiles

### Patch 1.1.6
- Pushed March 31, 2025 7:48pm ; Sam Holtzman
- Multiple QOL (Quality of Life) improvements
- Added win screen and win condition

### Patch 1.1.5
- Pushed March 27, 2025 2:42pm ; Sam Holtzman
- Undo button fixed

### Patch 1.1.4
- Pushed March 27, 2025 4:18am ; Sam Holtzman
- all peripherals implemented
- buttons need work 
  - size not large enough
  - functionality needs fixing 
    - Undo to only undo the last move made
    - pause to switch to play when it is pressed

### Patch 1.1.3
- Pushed March 25, 2025 10:11am ; Sam Holtzman
- scoreboards implemented. 

### Patch 1.1.2
- Pushed March 24, 2025 3:46pm ; Sam Holtzman
- repaired crashing but introduced while removing unused params in tile_view.py __init__()

### Patch 1.1.1
- Pushed March 24, 2025 2:48pm ; Sam Holtzman
- Game now playable
- resolved some warnings
- resolved some typos
- removed some redundant comments

### Patch 1.0.9
- Pushed March 24, 2025 2:42pm ; Sam Holtzman
- game_board.py updates; game is play-able
- settings_view.py updates; fixed settings margins
- Need to implement further UI items such as scoreboards, clock, etc

### Patch 1.0.8
- Pushed March 20, 2025 1:56am ; Sam Holtzman
- Significant UI logic advancements
- Move logic has started, but need to actually assess valid moves
- Need to allow for multi-select without crashing
- Need to implement further UI items such as scoreboards, clock, etc

### Patch 1.0.7
- Pushed March 13, 2025 11:15pm ; Wayne Chen
- Updated the code for reading input file in main.cpp

### Patch 1.0.6
- Pushed March 12, 2025 4:00pm ; Wayne Chen
- Updated cpp files to use namespace
- Renamed the output files in main.cpp
- Updated readme file

### Patch 1.0.5
- Pushed March 11, 2025 11:10am ; Wayne Chen
- Added a timer in main.cpp
- Added starting_position_input
- Added some edge_cases_input

### Patch 1.0.4
- Pushed March 9, 2025 07:55pm ; Wayne Chen
- Added board_visualizer.cpp
- Updated makefile with visualizer commands
- Updated readme with visualizer commands and formatting

### Patch 1.0.3
- Pushed March 9, 2025 11:36pm ; Wayne Chen
- Merged Logan's repo with Project repo
- Added makefile
- Updated readme with makefile commands

### Patch 1.0.2
- Pushed March 9, 2025 1:37am ; Sam Holtzman
- Added python directory with empty files
- Added initial UML diagram (in public directory)
- Moved C++ items into focussed directory
    - Suggest to move boards and other non C++ files into "resources" sub-directory inside cpp_directory 
        - Didnt want to do this incase current structure is necessary

### Patch 1.0.1
- Pushed March 7, 2025 9:06pm ; Wayne Chen
- Merged Logan's repo with Project repo
- Added patch notes back
- Updated gitignore

### Patch 1.0.0
- Pushed March 6, 2025 2:25pm ; Sam Holtzman
- Initial Project Setup
    -  Added file / directory structure
