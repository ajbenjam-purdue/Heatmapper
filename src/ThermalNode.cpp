#include <iostream>
#include <string>
#include <vector>
#include "ThermalNode.h"

// Constructors
ThermalNode::ThermalNode(size_t id)
    : canvas_position_x(0.5),
      canvas_position_y(0.5),
      property_mass(1.0),
      property_specific_heat(1000.0),
      property_label("Defualt"),
      node_id(id),
      node_temperature(15.0),
      ext_load(0.0)
{
}

ThermalNode::ThermalNode(double position_x, double position_y, double mass, double specific_heat, std::string label, size_t id, double temperature)
{
    canvas_position_x = position_x;
    canvas_position_y = position_y;
    property_mass = mass;
    property_specific_heat = specific_heat;
    property_label = label;
    node_id = id;
    node_temperature = temperature;
    ext_load = 0.0;
}

// Override Properties
void ThermalNode::setProperties(double mass, double specific_heat)
{
    property_mass = mass;
    property_specific_heat = specific_heat;
}

void ThermalNode::rename(std::string label)
{
    property_label = label;
}

void ThermalNode::fixTemperature(double temp)
{
    is_fixed_temperature = true;
    node_temperature = temp;
}

void ThermalNode::applyHeatLoad(double watts)
{
    ext_load = watts;
}

void ThermalNode::clearConstraints()
{
    is_fixed_temperature = false;
    ext_load= 0.0;
}

// Overload (printability)
std::ostream &operator<<(std::ostream &os, const ThermalNode &node)
{
    os << "Node \"" << node.property_label << "\" at position " << node.node_id << " (" << node.node_temperature << " C, " << node.property_mass << " kg)";
    return os;
}

// Returns (position x [px], position y [px])
std::pair<double, double> ThermalNode::screenCoordinates(double screen_width, double screen_height, double canvas_margin)
{
    return std::pair<double, double>(canvas_margin + (screen_width - 2 * canvas_margin) * canvas_position_x, canvas_margin + (screen_height - 2 * canvas_margin) * canvas_position_y);
}