# c_compiler

A C project created with the VSCode C Project Generator extension.

## Prerequisites

- CMake 3.15 or higher
- GCC/Clang C compiler
- vcpkg (optional, for package management)

## Building

### Using VSCode

1. Open the project folder in VSCode
2. Install the recommended extensions (C/C++, CMake Tools)
3. Press `Ctrl+Shift+P` and run "CMake: Configure"
4. Press `F7` or use "CMake: Build" to build the project

### Command Line

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./c_compiler
```

## Project Structure

```
c_compiler/
├── .vscode/              # VSCode configuration
├── src/                  # Source files
├── include/              # Header files
├── build/               # Build output (generated)
├── CMakeLists.txt       # CMake configuration
├── vcpkg.json          # vcpkg dependencies
├── .clang-format       # Code formatting rules
├── .editorconfig       # Editor configuration
├── .gitignore          # Git ignore rules
├── .gitattributes      # Git attributes
└── README.md           # This file
```

## Features

- Modern C standards (C17)
- CMake build system
- vcpkg integration for package management
- Clang-format for code formatting
- VSCode debugging configuration
- Cross-platform support (Windows, Linux, macOS)

## Adding Dependencies

Edit `vcpkg.json` to add dependencies:

```json
{
  "dependencies": [
    "fmt",
    "spdlog"
  ]
}
```

Then reconfigure CMake to install the dependencies.

## Debugging

Press `F5` in VSCode to start debugging, or use the "Debug" configuration in the Run and Debug panel.
