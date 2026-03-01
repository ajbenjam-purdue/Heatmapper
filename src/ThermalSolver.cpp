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
        if (node.is_fixed_temperature) {
            // Fixed temperature, apply gigantic apparent load to both the heat vector and conductivity matrix
            K(node.node_id, node.node_id) += 1.0e10;
            Q(node.node_id) = 1.0e10 * node.node_temperature;
        }
        else if (node.ext_load != 0.0) {
            Q(node.node_id) = node.ext_load;
        }
    }

    // Solve the system
    auto solve_start = std::chrono::steady_clock::now();
    Eigen::VectorXd T = K.colPivHouseholderQr().solve(Q);
    auto solve_complete = std::chrono::steady_clock::now();
    std::cout << "Solution found in " << std::chrono::duration<double, std::micro>(solve_complete - solve_start) << std::endl;

    // Write solution to network's nodes
    network.apply_temperatures(std::vector<double>(T.data(), T.data()+T.size()));
}