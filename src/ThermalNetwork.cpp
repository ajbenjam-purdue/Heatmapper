#include "ThermalNetwork.h"

ThermalNetwork::ThermalNetwork(std::string label, std::vector<ThermalNode> nodes, std::vector<ThermalEdge> edges)
    : network_edges(edges),
        network_nodes(nodes),
        network_label(label) {
    std::cout << "Created network " << label << std::endl;
}
