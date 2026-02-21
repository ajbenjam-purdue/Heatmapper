#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "ThermalNode.h"
#include "ThermalEdge.h"

class ThermalNetwork {
    public:

    // Init member nodes & edges
    std::vector<ThermalNode> network_nodes;
    std::vector<ThermalEdge> network_edges;

    std::string network_label;

    // Supplied vectors overload
    ThermalNetwork(std::string label, std::vector<ThermalNode> nodes, std::vector<ThermalEdge> edges);



    // Create and assign edges
    
};