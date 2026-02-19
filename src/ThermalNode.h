#pragma once
#include <iostream>
#include <string>
#include <vector>

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

    // Init state
    double node_temperature; // celcius

    ThermalNode(double position_x = 0.5, double position_y = 0.5, double mass = 1.0, double specific_heat = 500.0, std::string label = "Default Node", size_t id = 0, double temperature = 15.0);

    ThermalNode(double position_x, double position_y, double mass, double specific_heat, std::string label, size_t id, double temperature) {
        canvas_position_x = position_x;
        canvas_position_y = position_y;
        property_mass = mass;
        property_specific_heat = specific_heat;
        property_label = label;
        node_id = id;
        node_temperature = temperature;

        std::cout << "Created thermal node" << std::endl;
    }

    void setProperties(double mass = 1.0, double specific_heat = 500);

    void setProperties(double mass, double specific_heat) {
        property_mass = mass;
        property_specific_heat = specific_heat;
    }

    void tick(std::vector<double> fluxes, double delta_t) {
        double net_flux = 0.0;
        for (size_t i = 0; i < fluxes.size(); i++) {
            net_flux += fluxes.at(i);
        }
        node_temperature += net_flux * delta_t / (property_mass * property_specific_heat);
    }

    // TODO: Address SS behavior first. Then, finite time simulations
    // Node should track its own temperature and that's it
};
