# Heatmapper
### Version 0.1.6 | March 9, 2026
Cross platform GUI Heatmapper tool

## What is this, who is it for?

Heatmapper is a general utility I would've found useful a few years ago in my undergrad heat & mass transfer courses. It applies a network and edge approach, often referred to as a resistor network, to solve complex thermal circuits. While students analytically solve for fluxes, resistances, or temperatures, Heatmapper uses [Eigen](https://libeigen.gitlab.io) 3.5 (5.0.0) to quickly numerically solve for the same variables. Saving to human-readable files is done using [nlohmann's json header-only library](https://github.com/nlohmann/json), and materials information is sourced primarily from [ASM](asm.matweb.com). 

Since I use both a desktop and an M2 Macbook, it needed to be portable and cross-platform. It should run on any Apple Silicon Mac or any modern x64 Windows version, but I've only tested on my own M2 Macbook Pro and x64 Win10 PC. This is more of a proof-of-concept than anything, and is my first real C++ project and deviation from Python.

## Building the application

This application is build with Wx. In order to locally build the application you must have the Wx library build on your machine.

Create build dir (MacOS):
```sh
cmake -B build -DwxWidgets_CONFIG_EXECUTABLE=/PATH_TO_wxWidgets-x.x.x/build-osx/wx-config
```

Build application (MacOS/Win10):
```sh
cmake --build build
```