#include "ThermalSolver.h"
#include <chrono>

void ThermalSolver::solveSteadyState(ThermalNetwork &network)
{
    int N = network.get_node_count(); // Count of nodes in the network

    Eigen::MatrixXd K = Eigen::MatrixXd::Zero(N, N); // N x N Conductivity matrix
    Eigen::VectorXd Q = Eigen::VectorXd::Zero(N);    // N-length Heat Vector

    // Loop through each edge, calculating conductance
    for (ThermalEdge edge : network.network_edges)
    {
        double edge_conductance = 1.0 / edge.resistance();

        // Self-conductance
        K(edge.id_0, edge.id_0) += edge_conductance;
        K(edge.id_1, edge.id_1) += edge_conductance;

        // Cross-conductance
        K(edge.id_0, edge.id_1) -= edge_conductance;
        K(edge.id_1, edge.id_0) -= edge_conductance;
    }

    // Loop through each node to apply boundary conditions
    for (ThermalNode node : network.network_nodes)
    {
        // BC check
        if (node.is_fixed_temperature)
        {
            // Fixed temperature, apply gigantic apparent load to both the heat vector and conductivity matrix
            K(node.node_id, node.node_id) += 1.0e10;
            Q(node.node_id) = 1.0e10 * node.node_temperature;
        }
        else if (node.ext_load != 0.0)
        {
            Q(node.node_id) = node.ext_load;
        }
    }

    // Solve the system
    auto solve_start = std::chrono::steady_clock::now();
    Eigen::VectorXd T = K.colPivHouseholderQr().solve(Q);
    auto solve_complete = std::chrono::steady_clock::now();
    std::cout << "Solution found in " << std::chrono::duration<double, std::micro>(solve_complete - solve_start) << std::endl;

    // Write solution to network's nodes
    network.apply_temperatures(std::vector<double>(T.data(), T.data() + T.size()));
}

void ThermalSolver::runSimulation(ThermalNetwork &network, const SimulationConfig &config)
{
    int N = network.get_node_count();

    // Allocate memory
    Eigen::MatrixXd K = Eigen::MatrixXd::Zero(N, N);
    Eigen::VectorXd Q = Eigen::VectorXd::Zero(N);
    Eigen::VectorXd C_inv = Eigen::VectorXd::Zero(N);
    Eigen::VectorXd T_current = Eigen::VectorXd::Zero(N);

    // Populate K and Q
    for (ThermalEdge edge : network.network_edges)
    {
        double edge_conductance = 1.0 / edge.resistance();

        // Self-conductance
        K(edge.id_0, edge.id_0) += edge_conductance;
        K(edge.id_1, edge.id_1) += edge_conductance;

        // Cross-conductance
        K(edge.id_0, edge.id_1) -= edge_conductance;
        K(edge.id_1, edge.id_0) -= edge_conductance;
    }

    // Loop through each node to apply boundary conditions
    for (ThermalNode node : network.network_nodes)
    {
        // BC check
        if (node.is_fixed_temperature)
        {
            // Fixed temperature, apply gigantic apparent load to both the heat vector and conductivity matrix
            K(node.node_id, node.node_id) += 1.0e10;
            Q(node.node_id) = 1.0e10 * node.node_temperature;
        }
        else if (node.ext_load != 0.0)
        {
            Q(node.node_id) = node.ext_load;
        }
    }

    for (size_t i = 0; i < N; i++)
    {
        // Populate current temperatures
        T_current(i) = network.network_nodes[i].node_temperature;

        // Populate Capacitance Inverse (1.0 / (mass * specific_heat))
        double cap = network.network_nodes[i].property_mass * network.network_nodes[i].property_specific_heat;
        C_inv(i) = 1.0 / cap; // Pre-calc inverse to optimize loop perf
    }

    // Transient sim
    double current_time = 0.0;
    std::cout << "Starting transient simulation..." << std::endl;

    while (current_time < config.max_time)
    {

        Eigen::VectorXd T_old = T_current; // Fast Eigen copy

        // Pass the pre-allocated memory into the step
        solveTransientStep(network, config.delta_t, K, C_inv, Q, T_current);
        current_time += config.delta_t;

        // Check for steady state
        if (config.stop_on_steady_state)
        {

            // Find maximum residual
            double max_residual = (T_current - T_old).cwiseAbs().maxCoeff();
            if (max_residual <= config.residual_threshold)
            {

                // Steady state reached
                std::cout << "Steady state reached at t = " << current_time << "s\n";
                break;
            }
        }
    }

    // After the loop is done, update the actual nodes
    for (size_t i = 0; i < N; i++)
    {
        network.network_nodes[i].node_temperature = T_current(i);
    }
}

void ThermalSolver::solveTransientStep(ThermalNetwork &network, double delta_t, const Eigen::MatrixXd &K, const Eigen::VectorXd &C_inv, const Eigen::VectorXd &Q, Eigen::VectorXd &T_current)
{
    T_current = T_current + delta_t * C_inv.cwiseProduct(Q - (K * T_current));
}