#pragma once
#include "ThermalNetwork.h"
#include <Eigen/Dense>
#include <fstream>
#include <iomanip>

namespace ThermalSolver {

    struct SimulationConfig {
        bool stop_on_steady_state = false;  // Stop once SS attained? (TR)
        int max_ss_iterations = 100;        // Maximum Cycle Iterations (SS/TR)
        double ss_relaxation = 0.75;        // Relaxation Factor (SS/TR)
        double delta_t = 0.1;               // Time step size (seconds) (TR)
        double max_time = 5.0;              // Absolute maximum simulation time (TR)
        double residual_threshold = 1e-4;   // Upper limit for residuals (SS/TR)
        const std::atomic<bool>* stop_requested = nullptr; // Optional stop signal
        bool save_csv = true;               // Save the raw CSV
        bool save_compressed_csv = false;   // Save the compressed BZ2 CSV
    };

    // Steady State solver
    void solveSteadyState(ThermalNetwork& network, const SimulationConfig &config);

    // Transient solver
    void solveTransient(ThermalNetwork& network, const SimulationConfig& config, const std::string &save_path);
}