#pragma once
#include "ThermalNetwork.h"
#include <Eigen/Dense>

namespace ThermalSolver {

    struct SimulationConfig {
        bool stop_on_steady_state = false;  // Stop once SS attained? (TR)
        int max_ss_iterations = 100;        // Maximum Cycle Iterations (SS/TR)
        double ss_relaxation = 0.75;        // Relaxation Factor (SS/TR)
        double delta_t = 0.1;               // Time step size (seconds) (TR)
        double max_time = 5.0;              // Absolute maximum simulation time (TR)
        double residual_threshold = 1e-4;   // Upper limit for residuals (SS/TR)
    };

    // Pass the network by ref so the solver modifies the actual network
    void solveSteadyState(ThermalNetwork& network, SimulationConfig config); // SS solver

    // Transient solver
    void solveTransientStep(
        ThermalNetwork& network, 
        double delta_t,
        const Eigen::MatrixXd& K,
        const Eigen::VectorXd& C_inv, 
        const Eigen::VectorXd& Q,
        Eigen::VectorXd& T_current
    );
    
    void runSimulation(ThermalNetwork& network, const SimulationConfig& config);
}