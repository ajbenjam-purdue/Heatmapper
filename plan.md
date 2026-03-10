# Roadmap

No dates. Things will come when they come.

### Features

1. R-Click context menu
    
    - Discretize a single node into distributed chain/grid/network
    - Copy/paste node(s)
    - etc

2. Edge resistance menu

    - Select an edge resistance based on context: R'' + cross sectional area, simple convection, conduction, radiation, fin arrays, etc
    - Maybe look into natural convection, employ the use of Bergman et. al correlations
    - Unified, clean, dark svg graphics depicting the situation at hand (planar surface, fins, ...)
    - Scenarios: (Conduction) contact resistance (117), spherical shell (143), shape factors (235+); (Convection:External) Single fin, fin array flat plate in parallel flow, cylinder in cross flow, other shapes in cross flow, sphere (calculated or assumed h) (159+, 437, 455, 459, 465); (Radiation) very work-in-progress

3. Transient analysis

    - "Live" mode: click run and see how the circuit responds in real time. Need to do lots of learning for that. SCOPE EXPANDED: Radiation/Convection mandate potential iterative solutions, so need to create a solution wrapper that is context aware.
    - "Period" mode: runs analysis for (some amount of time). Outputs CSV for easier analysis.

4. Developer ease of use

    - Modularize the implementation of new resistance types, to boil down to essentially a list of inputs, assets (svg), and outputs.

### Fixes

[x] CRASH: After clearing a workspace, shift-clicking with the create node tool results in a segfault (TODO: CHECK ON THIS) Turned out it was an issue with MacOS cocoa's menu manager deleting the program instance when someone selected/deselected a ndoe twice quickly

[x] NONFUNCTIONAL: Confirmation for clearance of workspace doesnt work (N>4)

[x] BUG: Edges not loading/saving correctly