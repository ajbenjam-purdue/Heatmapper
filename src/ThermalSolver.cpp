#include "ThermalSolver.h"
#include <chrono>
#include <Eigen/Sparse>
#include <sstream>
#include <bzlib.h>

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

    Eigen::VectorXd Q = Eigen::VectorXd::Zero(N);    // N-length Heat Vector
    Eigen::VectorXd T_guess = Eigen::VectorXd::Zero(N); // N-length Temperature Guess Vector

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

    // Sparse Solver Instance
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;

    // Outer loop
    while (iter < max_iter && max_residual > tolerance)
    {
        // Check for stop signal
        if (config.stop_requested && config.stop_requested->load()) {
            std::cout << "Steady-state solver stopped by user." << std::endl;
            return;
        }

        // Use Triplets for efficient sparse assembly
        std::vector<Eigen::Triplet<double>> triplets;
        triplets.reserve(network.network_edges.size() * 4 + N);
        
        Q.setZero();

        // Loop through each edge, calculating conductance
        for (const ThermalEdge& edge : network.network_edges)
        {
            int idx_0 = id_to_index[edge.id_0];
            int idx_1 = id_to_index[edge.id_1];
            
            // Grab the guess temperatures of the two connected nodes
            double t1 = T_guess(idx_0);
            double t2 = T_guess(idx_1);

            // calculate conductance using the variant
            double cond = 1.0 / edge.resistance(t1, t2);

            // Add to triplets for K
            triplets.push_back(Eigen::Triplet<double>(idx_0, idx_0, cond));
            triplets.push_back(Eigen::Triplet<double>(idx_1, idx_1, cond));
            triplets.push_back(Eigen::Triplet<double>(idx_0, idx_1, -cond));
            triplets.push_back(Eigen::Triplet<double>(idx_1, idx_0, -cond));
        }

        // Loop through each node to apply boundary conditions and external loads
        for (auto const& [id, node] : network.network_nodes)
        {
            int idx = id_to_index[id];

            if (node.ext_load != 0.0) // External load
            {
                Q(idx) += node.ext_load;
            }
        }

        // Assemble Sparse Matrix K
        Eigen::SparseMatrix<double> K(N, N);
        K.setFromTriplets(triplets.begin(), triplets.end());

        // Refined approach: filter triplets
        std::vector<Eigen::Triplet<double>> filteredTriplets;
        filteredTriplets.reserve(triplets.size());
        
        std::unordered_set<int> fixed_indices;
        for (auto const& [id, node] : network.network_nodes) {
            if (node.is_fixed_temperature) fixed_indices.insert(id_to_index[id]);
        }

        for (const auto& trip : triplets) {
            if (fixed_indices.count(trip.row()) == 0) {
                filteredTriplets.push_back(trip);
            }
        }
        
        for (int idx : fixed_indices) {
            filteredTriplets.push_back(Eigen::Triplet<double>(idx, idx, 1.0));
            // actually find the correct node
        }

        // Robust BC handling
        filteredTriplets.clear();
        Q.setZero();
        for (const auto& trip : triplets) {
            if (fixed_indices.count(trip.row()) == 0) {
                filteredTriplets.push_back(trip);
            }
        }
        for (auto const& [id, node] : network.network_nodes) {
            int idx = id_to_index[id];
            if (node.is_fixed_temperature) {
                filteredTriplets.push_back(Eigen::Triplet<double>(idx, idx, 1.0));
                Q(idx) = node.node_temperature;
            } else {
                Q(idx) += node.ext_load;
            }
        }
        
        K.setFromTriplets(filteredTriplets.begin(), filteredTriplets.end());

        // Solve the system
        auto solve_start = std::chrono::steady_clock::now();
        solver.compute(K);
        if(solver.info() != Eigen::Success) {
            std::cerr << "Sparse decomposition failed!" << std::endl;
            return;
        }
        Eigen::VectorXd T_new = solver.solve(Q);
        auto solve_complete = std::chrono::steady_clock::now();

        // Calculate Residual & Apply Under Relaxation
        max_residual = (T_new - T_guess).cwiseAbs().maxCoeff();
        T_guess = (alpha * T_new) + ((1.0 - alpha) * T_guess);
        
        std::cout << "Solution for iteration " << iter << " found in " << std::chrono::duration<double, std::micro>(solve_complete - solve_start).count() << "us (Residuals = " << max_residual << ")\n";
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

void ThermalSolver::solveTransient(ThermalNetwork &network, const SimulationConfig &config, const std::string &save_path)
{
    double dt = config.delta_t;
    double total_time = config.max_time;
    int N = network.get_node_count();
    if (N == 0 || dt <= 0.0) return;

    // Map IDs to Matrix Indices
    std::unordered_map<int, int> id_to_index;
    int current_row = 0;
    for (const auto& [id, node] : network.network_nodes) {
        id_to_index[id] = current_row++;
    }

    // Build Capacitance Vector (C = mass * specific_heat)
    Eigen::VectorXd C = Eigen::VectorXd::Zero(N);
    Eigen::VectorXd T_old = Eigen::VectorXd::Zero(N);
    
    std::unordered_set<int> fixed_indices;
    for (const auto& [id, node] : network.network_nodes) {
        int idx = id_to_index[id];
        C(idx) = node.property_mass * node.property_specific_heat;
        T_old(idx) = node.node_temperature; // Seed initial temperatures
        if (node.is_fixed_temperature) fixed_indices.insert(idx);
    }

    // Use a stringstream to buffer all CSV data for potential compression
    std::stringstream csv_buffer;
    csv_buffer << "Time (s)";

    // Nodes
    for (int i = 0; i < N; ++i) {
        for (const auto& [id, node] : network.network_nodes) {
            if (id_to_index[id] == i) {
                csv_buffer << "," << node.property_label;
                if (node.is_fixed_temperature)
                    csv_buffer << " (Fixed)";
                else if (std::abs(node.ext_load) > 1e-6)
                    csv_buffer << (node.ext_load > 0 ? " (+" : " (-") << node.ext_load << "W)";
                csv_buffer << " [C]";
                break;
            }
        }
    }

    // Edges
    for (size_t i = 0; i < network.network_edges.size(); i++)
    {
        const ThermalEdge& edge = network.network_edges.at(i);
        csv_buffer << "," << network.network_nodes.at(edge.id_0).property_label << " -> " << network.network_nodes.at(edge.id_1).property_label << " [W]";
    }

    csv_buffer << "\n";

    // If we are saving raw CSV, open the file
    std::ofstream raw_csv_file;
    if (config.save_csv) {
        raw_csv_file.open(save_path);
        raw_csv_file << csv_buffer.str();
    }

    // Sparse Solver Instance
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;

    // Time Loop
    for (double time = 0; time <= total_time; time += dt) 
    {
        // Check for stop signal
        if (config.stop_requested && config.stop_requested->load()) {
            std::cout << "Transient solver stopped by user." << std::endl;
            break;
        }

        std::cout << "Time " << time << " / " << total_time << " (dt=" << dt << ")\n";
        
        // Build current state line
        std::stringstream line_buffer;
        line_buffer << std::fixed << std::setprecision(4) << time;
        for (int i = 0; i < N; ++i) line_buffer << "," << T_old(i); // Nodes
        for (size_t i = 0; i < network.network_edges.size(); i++)   // Edges
        {
            const ThermalEdge& edge = network.network_edges.at(i);
            double flux = (T_old(id_to_index[edge.id_1]) - T_old(id_to_index[edge.id_0])) / edge.resistance(T_old(id_to_index[edge.id_1]), T_old(id_to_index[edge.id_0]));
            line_buffer << "," << flux;
        }
        line_buffer << "\n";

        // Append to accumulation buffer
        csv_buffer << line_buffer.str();

        // Write to raw CSV if requested
        if (config.save_csv) {
            raw_csv_file << line_buffer.str();
        }

        // Build the Implicit Matrices A and b
        // A = [K] + [C]/dt
        // b = ([C]/dt){T_old} + {Q_ext}
        
        std::vector<Eigen::Triplet<double>> triplets;
        triplets.reserve(network.network_edges.size() * 4 + N);
        Eigen::VectorXd b = (1.0 / dt) * C.cwiseProduct(T_old);

        // Assemble [K] contributions to A
        for (const ThermalEdge& edge : network.network_edges) {
            int idx_0 = id_to_index[edge.id_0];
            int idx_1 = id_to_index[edge.id_1];
            
            double t1 = T_old(idx_0);
            double t2 = T_old(idx_1);
            double cond = 1.0 / edge.resistance(t1, t2);

            // Only add to row if it's NOT a fixed temperature node
            if (fixed_indices.count(idx_0) == 0) {
                triplets.push_back(Eigen::Triplet<double>(idx_0, idx_0, cond));
                triplets.push_back(Eigen::Triplet<double>(idx_0, idx_1, -cond));
            }
            if (fixed_indices.count(idx_1) == 0) {
                triplets.push_back(Eigen::Triplet<double>(idx_1, idx_1, cond));
                triplets.push_back(Eigen::Triplet<double>(idx_1, idx_0, -cond));
            }
        }

        // Add Capacitance and BCs
        for (auto const& [id, node] : network.network_nodes) {
            int idx = id_to_index[id];
            if (node.is_fixed_temperature) {
                triplets.push_back(Eigen::Triplet<double>(idx, idx, 1.0));
                b(idx) = node.node_temperature;
            } else {
                triplets.push_back(Eigen::Triplet<double>(idx, idx, C(idx) / dt));
                if (node.ext_load != 0.0) b(idx) += node.ext_load;
            }
        }

        Eigen::SparseMatrix<double> A(N, N);
        A.setFromTriplets(triplets.begin(), triplets.end());

        // Solve for T_new
        solver.compute(A);
        if(solver.info() != Eigen::Success) {
            std::cerr << "Transient sparse decomposition failed!" << std::endl;
            break;
        }
        T_old = solver.solve(b);
    }

    if (config.save_csv) {
        raw_csv_file.close();
        std::cout << "Saved raw CSV to " << save_path << "\n";
    }

    // bzip2 compression if requested
    if (config.save_compressed_csv) {
        std::string csv_data = csv_buffer.str();
        unsigned int in_size = (unsigned int)csv_data.size();
        
        // bzip2 output buffer should be approx 1.01 * in_size + 600 as per bz2 docs
        unsigned int out_size = (unsigned int)(in_size * 1.01) + 600; 
        std::vector<char> compressed_data(out_size);

        // 900k block size (theoretical best comp), default work factor of 30
        int result = BZ2_bzBuffToBuffCompress(
            compressed_data.data(), &out_size, 
            const_cast<char*>(csv_data.data()), 
            in_size, 9, 0, 30
        );

        if (result == BZ_OK) {
            std::ofstream compressed_file(save_path + ".bz2", std::ios::binary);
            compressed_file.write(compressed_data.data(), out_size);
            compressed_file.close();
            std::cout << "Saved compressed CSV to " << save_path << ".bz2" << "\n";
        } else {
            std::cerr << "bzip2 compression failed with error code: " << result << "\n";
        }
    }

    // Update the network with the very last computed temperatures
    for (auto& [id, node] : network.network_nodes) {
        if (!node.is_fixed_temperature) {
            node.node_temperature = T_old(id_to_index[id]);
        }
    }
    std::cout << "Updated network with transient results" << std::endl;
}