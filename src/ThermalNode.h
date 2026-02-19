#pragma once
#include <iostream>
#include <string>

class ThermalNode {
    public:

    // Init position
    double canvas_position_x;
    double canvas_position_y;

    // Init node ID (mapping)
    size_t node_id;

    // Init properties
    double property_mass; // kilograms
    double property_specific_heat; // joules per kg-K 
    std::string property_label; // What to label the node

    ThermalNode(double position_x = 0.5, double position_y = 0.5, double mass = 1.0, double specific_heat = 500.0, std::string label = "Default Node", size_t id = 0);

    ThermalNode(double position_x, double position_y, double mass, double specific_heat, std::string label, size_t id) {
        canvas_position_x = position_x;
        canvas_position_y = position_y;
        property_mass = mass;
        property_specific_heat = specific_heat;
        property_label = label;
        node_id = id;

        std::cout << "Created thermal node" << std::endl;
    }

    void setProperties(double mass = 1.0, double specific_heat = 500);

    void setProperties(double mass, double specific_heat) {
        property_mass = mass;
        property_specific_heat = specific_heat;
    }

    // TODO: Address SS behavior first. Then, finite time simulations
    // Node should track its own temperature and that's it
};
