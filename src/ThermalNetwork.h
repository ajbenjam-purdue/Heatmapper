#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "ThermalNode.h"
#include "ThermalEdge.h"

#include <json.hpp>
using json = nlohmann::json;

class ThermalNetwork {
    public:

    // Init member nodes & edges
    std::vector<ThermalNode> network_nodes;
    std::vector<ThermalEdge> network_edges;
    std::vector<int> network_node_ids;

    std::string network_label;

    // Supplied vectors overload
    ThermalNetwork(std::vector<ThermalNode> nodes, std::vector<ThermalEdge> edges, std::string label);

    // No vectors, just label
    ThermalNetwork(std::string label);

    // Create and assign node(s)
    void add_node(ThermalNode node);

    void add_nodes(std::vector<ThermalNode> nodes);

    // Create and assign edge(s)
    void add_edge(ThermalEdge edge);

    void add_edges(std::vector<ThermalEdge> edges);

    int get_node_count(void);

    void apply_temperatures(std::vector<double> temperatures);

    // Import/Export
    json to_json() const;
    
    static ThermalNetwork from_json(const json& j);
};