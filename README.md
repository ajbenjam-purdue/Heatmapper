# Heatmapper
### Version 0.1.8 | March 10, 2026
Cross platform GUI Heatmapper tool

## What is this, who is it for?

Heatmapper is a general utility I would've found useful a few years ago in my undergrad heat & mass transfer courses. It applies a network and edge approach, often referred to as a resistor network, to solve complex thermal circuits. While students analytically solve for fluxes, resistances, or temperatures, Heatmapper uses [Eigen](https://libeigen.gitlab.io) 3.5 (5.0.0) to quickly numerically solve for the same variables. Saving to human-readable files is done using [nlohmann's json header-only library](https://github.com/nlohmann/json), and materials information is sourced primarily from [ASM](asm.matweb.com), or my reference textbook, _Fundamentals of Heat And Mass Transfer_ (Bergman et. al). 

Since I use both a desktop and an M2 Macbook, it needed to be portable and cross-platform. It should run on any Apple Silicon Mac or any modern x64 Windows version, but I've only tested on my own M2 Macbook Pro and x64 Win10 PC. This is more of a proof-of-concept than anything, and is my first real C++ project and deviation from Python.

## How to use the application

Once the Heatmapper.app or Heatmapper.exe is successfully built, the application is intuitive to understand and to use. A workspace will open and pre-populate with a few nodes connected by an edge. The nodes represent a chunk of mass, while the edge represents a physical thermal resistance between them. Four tools currently exist in the application: Select, Create Node, Create Edge, and Delete. They're self-explanitory:

 - **Select Tool**: Selects an Edge or Node(s) to edit. For nodes, supports copy+paste functionality and multi-select. With an Edge selected, its parameters (thermal resistance) can be configured and its measured flux can be observed. With a Node selected, its parameters (external flux, temperature B.C., physical parameters) can be configured and its measured temperature can be observed. Critically, _the user must select "Apply Changes"_ to save them to the Node(s)/Edge and update the network.
 - **Create Node**: Creates a new Node with default parameters and auto-selects it for the user. If shift is held, multiple nodes can be created.
 - **Create Edge**: Similar to Create Node, but with no multiple-creation support. Click once on the first node, and once on the second node. Since direction is irrelevant for resistors in a thermal network, the order in which a user defines the nodes to connect with an edge does not matter.
 - **Delete**: Deletes whatever the user clicks on.

**Saving** and **Loading** is done via the menubar `File > Save as .json` and `File > Load from .json`, respectively. While the `.json` files are human-readable, you should only allow the program to alter a circuit to maintain ID continuity within the network.

As of 0.1.6, a thermal resistance sub-menu has been introduced: all the user needs to do to easily calculate effective thermal resistance for a real scenario is click the _Open Edge Configuration Tool_ button. Alternatively, they may simply enter a thermal resistance when an Edge is selected with the Select Tool.

A primitive materials library is being built up with the application and is managed by the application. Eventually the user will be able to create custom materials.

## Building the application

This application is built with Wx. In order to locally build the application you must have the Wx library build on your machine. Additionally, you will need to change the paths in `CMakeLists.txt` to local instances of jlohmann/json and Eigen. Any modern version of these libraries should work, and neither has to be built for your specific system (unlike Wx).

Create build dir (MacOS):
```sh
cmake -B build -DwxWidgets_CONFIG_EXECUTABLE=/PATH_TO_wxWidgets-x.x.x/build-osx/wx-config
```

Build application (MacOS/Win10):
```sh
cmake --build build
```

## Changelog

### 0.1.8

 - Implemented changes to the underlying ThermalNetwork and ThermalEdge classes to add non-linear solution possibilities. 
 - Created a stand-in radiation resistance type as a POC, which still needs assets.
 - Fully reworked backend of ThermalEdge EdgeConfig dialog to enable easier development of new configurations.

### 0.1.7

 - Added substantial feature: Node discretization, allowing a user to split a single node into a network representing real things. For example, one node may be split into a comb graph representing a jacketed pipe, with a discretized representation of both axial and radial flux. 
 - Fixed a persistent MacOS crash with empty networks and corrected network saving/loading behavior.

### 0.1.6 and Earlier
 - Check commit history