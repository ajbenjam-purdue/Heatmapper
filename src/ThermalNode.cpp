#include <iostream>
#include <string>
#include <vector>
#include "ThermalNode.h"

// Constructor
ThermalNode::ThermalNode(double position_x, double position_y, double mass, double specific_heat, std::string label, size_t id, double temperature) {
    canvas_position_x = position_x;
    canvas_position_y = position_y;
    property_mass = mass;
    property_specific_heat = specific_heat;
    property_label = label;
    node_id = id;
    node_temperature = temperature;

    std::cout << "Created thermal node" << std::endl;
}

// Override Properties
void ThermalNode::setProperties(double mass, double specific_heat) {
    property_mass = mass;
    property_specific_heat = specific_heat;
}

void ThermalNode::tick(std::vector<double> fluxes, double delta_t) {
    double net_flux = 0.0;
    for (size_t i = 0; i < fluxes.size(); i++) {
        net_flux += fluxes.at(i);
    }
    node_temperature += net_flux * delta_t / (property_mass * property_specific_heat);
}

void ThermalNode::rename(std::string label) {
    property_label = label;
}

// Overload (printability)
std::ostream& operator<<(std::ostream& os, const ThermalNode& node) {
    os << "Node \"" << node.property_label << "\" at position " << node.node_id << " (" << node.node_temperature << " C, " << node.property_mass << " kg)";
    return os;
}