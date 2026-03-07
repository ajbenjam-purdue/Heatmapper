#pragma once
#include <iostream>
#include <string>
#include <vector>

class ThermalNode
{
public:
    // Init position
    double canvas_position_x;
    double canvas_position_y;

    // Init node ID (mapping)
    size_t node_id;

    // Init properties
    double property_mass;          // kilograms
    double property_specific_heat; // joules per kg-K
    std::string property_label;    // What to label the node

    // Init state
    double node_temperature; // celcius
    bool is_fixed_temperature; // Is the node fixed?
    double ext_load; // W, to simulate heat flux impositions

    // Default node with id
    ThermalNode(size_t id);

    // Node with params
    ThermalNode(double position_x = 0.5, double position_y = 0.5, double mass = 1.0, double specific_heat = 500.0, std::string label = "Default Node", size_t id = 0, double temperature = 15.0);

    void setProperties(double mass = 1.0, double specific_heat = 500);

    void rename(std::string label);

    void fixTemperature(double temp);

    void applyHeatLoad(double watts);

    void clearConstraints();

    std::pair<double, double> screenCoordinates(double screen_width, double screen_height, double canvas_margin);
};

std::ostream &operator<<(std::ostream &os, const ThermalNode &node);