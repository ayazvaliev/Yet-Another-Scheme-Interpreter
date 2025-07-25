
# YAScI - Yet Another Scheme Interpreter 🌀
A lightweight basic Scheme interpreter written in modern C++20 (Implemented as part of the YSDA C++ course)

## 📝 Description

**YAScI** is a basic interpreter for the [Scheme](https://en.wikipedia.org/wiki/Scheme_(programming_language)) programming language, implemented from scratch in C++. It supports a subset of R5RS Scheme, including:

- Arithmetic and comparison operations
- Variable bindings (`define`)
- First-class functions and lambdas
- Conditionals (`if`)
- Recursion and closures
- Basic list operations
- A **Mark-and-Sweep Garbage Collector** for automatic memory management
- Modular architecture with separate parser, evaluator, and memory manager

## Usage

### 🔧 Prerequisites

- A C++20-compatible compiler (e.g., `g++ >= 10`, `clang++ >= 11`)
- [CMake ≥ 3.14](https://cmake.org/)
- A Unix-like terminal (Linux/macOS/WSL) or PowerShell (Windows)

### 🛠️ Building the Interpreter

```bash
git clone https://github.com/ayazvaliev/YAScI-Yet-Another-Scheme-Interpreter
cd YAScI-Yet-Another-Scheme-Interpreter
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### ▶️ Running the Interpreter
After building, you can run the interpreter interactively:

```bash
# Start the REPL
./build/scheme-interpreter
```

### 🔧 To Build and Run Tests

This project uses Catch2 for unit testing. Tests are located in the tests/ directory (can also be viewed as examples).

```bash

# Enable testing in CMake
cmake -B build -DENABLE_TESTING=ON

# Build everything (including test binaries)
cmake --build build

# Run tests with CTest
cd build
ctest --verbose

# Or run test binary directly
./tests/MyTests

```

### 🧾 Adding New Tests

Add a file like `test_parser.cpp` in `tests/`:

```c++
#include <catch2/catch_test_macros.hpp>
#include "Parser.hpp"

TEST_CASE_METHOD(SchemeTest, "Parses simple addition, catches list-related syntax errors") {
    ExpectEq("(+ 1 2)", "3");
    ExpectSyntaxError("(1 . 0 1)");
}
```

And rebuild with `-DENABLE_TESTING=ON`.