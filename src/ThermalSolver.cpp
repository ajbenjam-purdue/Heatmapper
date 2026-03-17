#include "ThermalSolver.h"
#include <chrono>

void ThermalSolver::solveSteadyState(ThermalNetwork &network, const SimulationConfig &config)
{
    int N = network.get_node_count(); // Count of nodes in the network
    if (N == 0) // No nodes
        return;

    // Map to map the unordered map to matrices
    std::unordered_map<int, int> id_to_index;
    int current_row = 0;

    for (const auto& [node_id, node] : network.network_nodes) {
        id_to_index[node_id] = current_row;
        current_row++;
    }

    Eigen::MatrixXd K = Eigen::MatrixXd::Zero(N, N); // N x N Conductivity matrix
    Eigen::VectorXd Q = Eigen::VectorXd::Zero(N);    // N-length Heat Vector
    Eigen::VectorXd T_guess = Eigen::VectorXd::Zero(N); // N x N Temperature Guess Vector

    // Seed initial guess with current temperatures
    for (const auto& [id, node] : network.network_nodes) {
        T_guess(id_to_index[id]) = node.node_temperature;
    }

    // Iteration trackers
    int iter = 0;
    int max_iter = config.max_ss_iterations;
    double tolerance = config.residual_threshold;
    double max_residual = 1.0; // Celcius placeholder
    double alpha = config.ss_relaxation;

    // Outer loop
    while (iter < max_iter && max_residual > tolerance)
    {
        // Reset matrices
        K.setZero();
        Q.setZero();

        // Loop through each edge, calculating conductance
        for (ThermalEdge edge : network.network_edges)
        {
            int idx_0 = id_to_index[edge.id_0];
            int idx_1 = id_to_index[edge.id_1];
            
            // Grab the guess temperatures of the two connected nodes
            double t1 = T_guess(idx_0);
            double t2 = T_guess(idx_1);

            // calculate conductance using the variant
            double cond = 1.0 / edge.resistance(t1, t2);

            // Populate K using the translated indices
            K(idx_0, idx_0) += cond;
            K(idx_1, idx_1) += cond;
            K(idx_0, idx_1) -= cond;
            K(idx_1, idx_0) -= cond;
        }

        // Loop through each node to apply boundary conditions
        for (auto const [id, node] : network.network_nodes)
        {
            // Translation
            int idx = id_to_index[id];

            // BC check
            if (node.is_fixed_temperature) // Fixed temperature
            {
                // Fixed temperature, apply gigantic apparent load to both the heat vector and conductivity matrix
                K.row(idx).setZero();
                K(idx, idx) = 1.0;
                Q(idx) = node.node_temperature;
            }
            else if (node.ext_load != 0.0) // External load
            {
                Q(idx) = node.ext_load;
            }
        }

        // Solve the system
        auto solve_start = std::chrono::steady_clock::now();
        Eigen::VectorXd T_new = K.colPivHouseholderQr().solve(Q);
        auto solve_complete = std::chrono::steady_clock::now();

        // Calculate Residual & Apply Under Relaxation
        max_residual = (T_new - T_guess).cwiseAbs().maxCoeff();
        T_guess = (alpha * T_new) + ((1.0 - alpha) * T_guess);
        
        std::cout << "Solution for iteration " << iter << " found in " << std::chrono::duration<double, std::micro>(solve_complete - solve_start) << " (Residuals = " << max_residual << ")\n";
        iter++;
    }

    // Write solution to network's nodes
    std::cout << "Converged in " << iter << " iterations." << std::endl;

    for (auto& [node_id, node] : network.network_nodes) {
        if (!node.is_fixed_temperature) {
            int idx = id_to_index[node_id]; 
            node.node_temperature = T_guess(idx); // Save the converged answer
        }
    }
}

void ThermalSolver::solveTransient(ThermalNetwork &network, const SimulationConfig &config, std::string &save_path)
{
    
}