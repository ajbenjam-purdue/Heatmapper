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

3. Transient analysis

    - "Live" mode: click run and see how the circuit responds in real time. Need to do lots of learning for that.
    - "Period" mode: runs analysis for (some amount of time). Outputs CSV for easier analysis.

### Fixes

[ ] CRASH: After clearing a workspace, shift-clicking with the create node tool results in a segfault (TODO: CHECK ON THIS)

[x] NONFUNCTIONAL: Confirmation for clearance of workspace doesnt work (N>4)

[ ] BUG: Edges not loading/saving correctly