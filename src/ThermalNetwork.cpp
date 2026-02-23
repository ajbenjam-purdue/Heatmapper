#include "ThermalNetwork.h"
#include <string>
#include <algorithm>

// Pre-populated network
ThermalNetwork::ThermalNetwork(std::vector<ThermalNode> nodes, std::vector<ThermalEdge> edges, std::string label)
    : network_nodes(nodes),
      network_edges(edges),
      network_label(label)
{

    // Populate nodes vector
    for (size_t i = 0; i < nodes.size(); i++)
    {
        network_node_ids.push_back(network_nodes.at(i).node_id);
    }
    std::cout << "Created network " << label << std::endl;
}

// Empty network
ThermalNetwork::ThermalNetwork(std::string label)
    : network_label(label)
{
    std::cout << "Initialized empty network " << label << std::endl;
}

// Add node, update network
void ThermalNetwork::add_node(ThermalNode node)
{
    network_nodes.push_back(node);
    network_node_ids.push_back(node.node_id);
}

// Add nodes, update network
void ThermalNetwork::add_nodes(std::vector<ThermalNode> nodes)
{
    for (size_t i = 0; i < nodes.size(); i++)
    {
        network_nodes.push_back(nodes.at(i));
        network_node_ids.push_back(nodes.at(i).node_id);
    }
}

// Add edge, checking for validity
void ThermalNetwork::add_edge(ThermalEdge edge)
{
    // Check for both ids present
    if (std::count(network_node_ids.begin(), network_node_ids.end(), edge.id_0) > 0 && std::count(network_node_ids.begin(), network_node_ids.end(), edge.id_1) > 0)
    {
        // Both present, add edge
        network_edges.push_back(edge);
    }
    else
    {
        // One+ missing, do nothing
        std::cout << "Edge " << edge.type << " connects to at least one non-existent node, not added. " << std::endl;
    }
}

// Add edges, checking for validity
void ThermalNetwork::add_edges(std::vector<ThermalEdge> edges)
{
    for (size_t i = 0; i < edges.size(); i++)
    {
        // Check for both ids present
        if (std::count(network_node_ids.begin(), network_node_ids.end(), edges.at(i).id_0) > 0 && std::count(network_node_ids.begin(), network_node_ids.end(), edges.at(i).id_1) > 0)
        {
            // Both present, add edge
            network_edges.push_back(edges.at(i));
        }
        else
        {
            // One+ missing, do nothing
            std::cout << "Edge " << edges.at(i).type << " connects to at least one non-existent node, not added. " << std::endl;
        }
    }
}