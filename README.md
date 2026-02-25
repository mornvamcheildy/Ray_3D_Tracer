# Ray 3D Tracer

A high-performance 3D ray tracer built with C++, utilizing multi-threading for efficient rendering.

# 🚀 Getting Started
Follow these instructions to compile and run the project on your local machine. 
Prerequisites
A C++ compiler (e.g., g++)
Support for POSIX threads (pthread)

# Compilation

To compile the project with optimizations enabled, use the following command in your terminal:

```bash
g++ -O3 -pthread main.cpp -o ray_tracer
```

# Usage

When running the executable, you must provide a light contrast value (e.g., 0.5) as a command-line argument: 

./ray_tracer 0.5

# 🛠 Features

<Multi-threaded: Uses -pthread to leverage multi-core processors.

<Optimized: Compiled with -O3 for maximum execution speed.

<Customizable Lighting: Adjust scene contrast directly from the terminal.