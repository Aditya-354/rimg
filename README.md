# Basic image rendering tool written in C++.

## Usage:

./build/rimg <path to image file>

> Note: You can alias the rimg executable path to access it just like any other program, or put it directly under /usr/bin/.

## Building the project:

To build the project, paste this block of code into the terminal:

```bash
git clone https://github.com/Aditya-354/rimg ~/
cd ~/rimg
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```
