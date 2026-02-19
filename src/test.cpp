#include <iostream>
#include "ThermalNode.h"
#include "ThermalEdge.h"

int main() {
    ThermalNode Node_A(0.5, 0.5, 1.0, 500.0, "Node A", 0, 15.0);
    ThermalNode Node_B(0.4, 0.4, 1.0, 350.0, "Node B", 1, 15.0);
    std::cout << Node_A << std::endl;
    std::cout << Node_B << std::endl;

    ThermalEdge Edge(0, 1, PureResistance{1.0});
    std::cout << Edge << std::endl;
}