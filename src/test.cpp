#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <json.hpp>
#include "ThermalNode.h"
#include "ThermalEdge.h"
#include "ThermalNetwork.h"
#include "ThermalSolver.h"

int main() {

    // Build nodes
    std::vector<ThermalNode> nodes;
    size_t linear_count = 4;
    for (size_t i = 0; i < linear_count; i++) {
        std::stringstream label;
        label << "Node " << std::to_string(i) << " / " << std::to_string(linear_count);
        nodes.push_back(ThermalNode((double)(i / (linear_count-1)), 0.5, 1.0, 1000.0, label.str(), i));
    }
    nodes.at(0).fixTemperature(15.0); // Approximate heatsink
    nodes.at(3).applyHeatLoad(200.0); // Approximate CPU

    // Build edges
    std::vector<ThermalEdge> edges;
    for (size_t i = 0; i < linear_count - 1; i++) {
        edges.push_back(ThermalEdge(i, i+1, PureResistance{0.1}));
    }

    // Create network
    ThermalNetwork network(nodes, edges, "network");

    ThermalSolver::solveSteadyState(network); // Solve

    json j;
    j = network.to_json();
    std::ofstream o("test.json");
    if (o.is_open()) {
        o << std::setw(4) << j << std::endl;
    }
}