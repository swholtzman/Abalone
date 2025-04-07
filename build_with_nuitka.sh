#!/bin/bash

# Exit on error
set -e

# Absolute path to the PIL .dylibs folder
DYLIB_DIR="/Users/ilagko/Abalone/venv/lib/python3.13/site-packages/PIL/.dylibs"

# Start building the command using an array for proper spacing/escaping
CMD=(
  nuitka
  --standalone
  --onefile
  --enable-plugin=pyqt5
  --macos-create-app-bundle
  --macos-app-icon=none
  --include-package=python_gui
  --include-data-files="/Users/ilagko/Abalone/venv/lib/python3.13/site-packages/PIL/*.py=PIL/"
  --include-data-files="/Users/ilagko/Abalone/venv/lib/python3.13/site-packages/PIL/_imaging*.so=PIL/"
  --include-data-files="build/abalone_ai*.so=."
)

# Add each .dylib file explicitly
for dylib in "$DYLIB_DIR"/*.dylib; do
  CMD+=(--include-data-files="$dylib=PIL/.dylibs/")
done

# Add the main entry point
CMD+=(python_gui/main_app.py)

# Print and run
echo "Running:"
echo "${CMD[@]}"

"${CMD[@]}"