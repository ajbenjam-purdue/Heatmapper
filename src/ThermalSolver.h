#pragma once
#include "ThermalNetwork.h"
#include <Eigen/Dense>

namespace ThermalSolver {
    // Pass the network by ref so the solver modifies the actual network
    void solveSteadyState(ThermalNetwork& network); // SS solver

    // Transient solver
    void solveTransientStep(
        ThermalNetwork& network, 
        double delta_t,
        const Eigen::MatrixXd& K,
        const Eigen::VectorXd& C_inv, 
        const Eigen::VectorXd& Q,
        Eigen::VectorXd& T_current
    );

    struct SimulationConfig {
        double delta_t = 0.1;              // Time step size (seconds)
        double max_time = 5.0;             // Absolute maximum simulation time
        
        bool stop_on_steady_state = false; // Residuals check
        double residual_threshold = 1e-4;  // Upper limit for residuals for SS
    };
    
    void runSimulation(ThermalNetwork& network, const SimulationConfig& config);
}