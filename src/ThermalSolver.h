#pragma once
#include "ThermalNetwork.h"
#include <Eigen/Dense>

namespace ThermalSolver {
    // Pass the network by ref so the solver modifies the actual network
    void solveSteadyState(ThermalNetwork& network);
}