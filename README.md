# Throughput Visualizer (C & Raylib)

A real-time visualization tool written in pure C for modeling and profiling system throughput (e.g., pipeline stages in computer architecture).

## Overview

This tool was developed to visually model the theoretical maximum throughput of a pipelined system based on varying parameters such as base time, overhead, and branch penalties. It utilizes an immediate-mode GUI to allow for live parameter tuning, instantly reflecting changes on the graphed function.

## Key Features

*   **Real-time Rendering:** Achieves a consistent 60 FPS utilizing a dedicated game-loop architecture.
*   **Live Parameter Tuning:** Immediate-mode GUI (via Raygui) allows for dynamic adjustment of mathematical parameters via direct pointer referencing.
*   **Dynamic Scaling:** The coordinate system and plotted spline dynamically scale based on window resizing and parameter bounds.
*   **Interactive Inspection:** Hover over the graph to inspect exact values at specific points along the curve.
*   **Memory Safe:** Strict manual memory management ensures no leaks during runtime or upon exit.

## Mathematical Model

The plotted function calculates the theoretical throughput $G$ given $S$ pipeline stages:

$$ G = \frac{1}{T} \cdot \frac{1}{1 + (S - k) \cdot b} \cdot \frac{S}{1 + (S - 1) \cdot \frac{C}{T}} $$

**Parameters:**
*   $T$: Base Time
*   $C$: Overhead per stage
*   $k$: Penalty stages
*   $b$: Branch penalty factor

## Getting Started

### Prerequisites

Ensure you have a standard C compiler (`gcc` or `clang`) and the necessary build tools installed.

### Dependencies

This project requires **Raylib**.

*   **Linux / WSL:** `sudo apt install libraylib-dev`
*   **macOS:** `brew install raylib`

### Building the Project

A `Makefile` is included for streamlined building. Navigate to the project directory and run:

    make

To compile and immediately execute the program, use:

    make run

To clean the compiled binaries:

    make clean

## Usage Controls

*   **[D]:** Toggle the parameter tuning UI on/off.
*   **[I]:** Toggle Inspect Mode (hover for exact values).
*   **[M]:** Toggle Theoretical Maximum indicator.
*   **[R]:** Toggle coordinate rounding.

## Technical Architecture

*   **Language:** C
*   **Graphics Backend:** Raylib (OpenGL)
*   **UI Framework:** Raygui (Immediate Mode)
*   **Memory:** Dynamic allocation via `malloc` and `free` for vertex data.
