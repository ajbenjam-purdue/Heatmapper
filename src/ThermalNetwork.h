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
    std::unordered_map<int, ThermalNode> network_nodes;
    std::vector<ThermalEdge> network_edges;
    int next_node_id = 0;

    std::string network_label;

    // Supplied vectors overload
    ThermalNetwork(std::vector<ThermalNode> nodes, std::vector<ThermalEdge> edges, std::string label);

    // No vectors, just label
    ThermalNetwork(std::string label);

    // Create and assign node(s)
    int add_node(ThermalNode node);

    void add_nodes(std::vector<ThermalNode> nodes);

    // Create and assign edge(s)
    void add_edge(ThermalEdge edge);

    void add_edges(std::vector<ThermalEdge> edges);

    int get_node_count(void);

    void apply_temperatures(std::vector<double> temperatures);

    // Import/Export
    json to_json() const;
    
    static ThermalNetwork from_json(const json& j);

    // Diagnostics
    double highest_node_temperature();
    
    double lowest_node_temperature();
    
    int nodes_with_temperature_fix(); // Count of nodes with a fixed temperature

    int nodes_with_flux_fix(); // Count of nodes with an input/output flux

    double get_edge_flux(size_t id); // Gets the flux across an edge
};