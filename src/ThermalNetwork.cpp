#include "ThermalNetwork.h"
#include <string>
#include <algorithm>

// Pre-populated network
ThermalNetwork::ThermalNetwork(std::vector<ThermalNode> nodes, std::vector<ThermalEdge> edges, std::string label)
    : network_edges(edges),
      network_label(label)
{

    // Populate nodes vector
    for (size_t i = 0; i < nodes.size(); i++)
    {
        network_nodes[next_node_id] = nodes.at(i);
        next_node_id++;
    }
    
    std::cout << "Created network \"" << label << "\" with nodes and vectors" << std::endl;
}

// Empty network
ThermalNetwork::ThermalNetwork(std::string label)
    : network_label(label)
{
    std::cout << "Initialized empty network \"" << label << "\"" << std::endl;
}

// Add node, update network
int ThermalNetwork::add_node(ThermalNode node)
{
    // Rework to prevent deletion crash
    int assigned_id = next_node_id++;
    ThermalNode n = node;
    n.node_id = assigned_id;
    network_nodes[assigned_id] = n;
    return assigned_id;
}

// Add nodes, update network
void ThermalNetwork::add_nodes(std::vector<ThermalNode> nodes)
{
    for (size_t i = 0; i < nodes.size(); i++)
    {
        nodes.at(i).node_id = next_node_id;
        network_nodes[next_node_id] = nodes.at(i);
        next_node_id++;
    }
}

// Add edge, checking for validity
void ThermalNetwork::add_edge(ThermalEdge edge)
{
    // Check for both ids present
    if (network_nodes.count(edge.id_0) > 0 && network_nodes.count(edge.id_1) > 0)
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
        if (network_nodes.count(edges.at(i).id_0) > 0 && network_nodes.count(edges.at(i).id_1) > 0)
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

int ThermalNetwork::get_node_count(void)
{
    return network_nodes.size();
}

void ThermalNetwork::apply_temperatures(std::vector<double> temperatures)
{
    for (size_t i = 0; i < temperatures.size(); i++)
    {
        network_nodes.at(i).node_temperature = temperatures.at(i);
    }
}

json ThermalNetwork::to_json() const {
    json j;
    j["label"] = network_label;
    
    // Order nodes
    j["nodes"] = json::array(); // explicitly create an array
    for (auto const& [id, node] : network_nodes) {
        j["nodes"].push_back({
            {"id", node.node_id},
            {"x", node.canvas_position_x},
            {"y", node.canvas_position_y},
            {"mass", node.property_mass},
            {"cp", node.property_specific_heat},
            {"label", node.property_label},
            {"temperature", node.node_temperature},
            {"is_fixed", node.is_fixed_temperature},
            {"load", node.ext_load}
        });
    }

    // Order network edges
    j["edges"] = json::array();
    for (const ThermalEdge& edge : network_edges) {
        json edge_json = {
            {"id_0", edge.id_0},
            {"id_1", edge.id_1},
            {"type", static_cast<int>(edge.type)} // Store the enum as an int
        };

        // Extract the variant data based on the type
        if (edge.type == EdgeType::RESISTANCE_PURE) {
            edge_json["R"] = std::get<PureResistance>(edge.params).R;
        } 
        // TODO: Other resistance types

        j["edges"].push_back(edge_json);
    }

    return j;
}

ThermalNetwork ThermalNetwork::from_json(const json& j) {
    // Read label and create an empty network
    std::string label = j.value("label", "Imported Network");
    ThermalNetwork new_network(label);

    // Deserialize nodes in correct order
    if (j.contains("nodes")) {
        for (const auto& node_json : j["nodes"]) {
            // Reconstruct the node obj
            ThermalNode node(
                node_json["x"], node_json["y"], 
                node_json["mass"], node_json["cp"], 
                node_json["label"], node_json["id"], 
                node_json["temperature"]
            );
            
            // Apply constraints
            if (node_json["is_fixed"]) {
                node.fixTemperature(node_json["temperature"]);
            } else if (node_json["load"] != 0.0) {
                node.applyHeatLoad(node_json["load"]);
            }

            // dodge add_node() to preserve the exact historical ID
            int original_id = node_json["id"];
            new_network.network_nodes[original_id] = node;

            // Push the network's internal ID counter safely past this node
            if (original_id >= new_network.next_node_id) {
                new_network.next_node_id = original_id + 1;
            }
        }
    }

    // Deserialize Edges
    if (j.contains("edges")) {
        for (const auto& edge_json : j["edges"]) {
            size_t id_0 = edge_json["id_0"];
            size_t id_1 = edge_json["id_1"];
            int type_int = edge_json["type"];
            EdgeType type = static_cast<EdgeType>(type_int);

            if (type == EdgeType::RESISTANCE_PURE) {
                PureResistance p { edge_json["R"] };
                new_network.add_edge(ThermalEdge(id_0, id_1, p));
            }
            // TODO: other resistance types
        }
    }

    return new_network;
}


double ThermalNetwork::highest_node_temperature()
{
    // Catch empty network, yield 0.0
    if (network_nodes.size() == 0)
        return 0.0;
    
    // Network has at least one element
    double res = network_nodes.at(0).node_temperature; // Initialize to first node temperature
    for (auto const& [id, node] : network_nodes)
    {
        res = std::max(res, node.node_temperature);
    }
    return res;
}

double ThermalNetwork::lowest_node_temperature() 
{
    // Catch empty network, yield 0.0
    if (network_nodes.size() == 0)
        return 0.0;
    
    // Network has at least one element
    double res = network_nodes.at(0).node_temperature; // Initialize to first node temperature
    for (auto const& [id, node] : network_nodes)
    {
        res = std::min(res, node.node_temperature);
    }
    return res;
}

// Count of nodes with a fixed temperature
int ThermalNetwork::nodes_with_temperature_fix()
{
    int res = 0;
    for (auto const& [id, node] : network_nodes)
    {
        res += (node.is_fixed_temperature ? 1 : 0);
    }
    return res;
}

// Count of nodes with an input/output flux
int ThermalNetwork::nodes_with_flux_fix() 
{
    int res = 0;
    for (auto const& [id, node] : network_nodes)
    {
        res += (node.ext_load != 0.0 ? 1 : 0);
    }
    return res;
}

// Gets the flux across an edge
double ThermalNetwork::get_edge_flux(size_t id) 
{
    ThermalNode& node_a = network_nodes[network_edges[id].id_0];
    ThermalNode& node_b = network_nodes[network_edges[id].id_1];
    double resistance = network_edges[id].resistance();
    return (node_b.node_temperature - node_a.node_temperature) / resistance;
}

// True if a matching edge exists
bool ThermalNetwork::has_edge(size_t id_0, size_t id_1)
{
    for (const ThermalEdge edge : network_edges)
    {
        if (std::min(edge.id_0, edge.id_1) == std::min(id_0, id_1) && std::max(edge.id_0, edge.id_1) == std::max(id_0, id_1))
            return true;
    }
    return false;
}

// Remove all nodes safely from network
void ThermalNetwork::network_nodes_clear()
{
    network_nodes.clear();
    next_node_id = 0; // Reset
    std::unordered_map<int, ThermalNode>().swap(network_nodes); // Swap with empty UM to clear buffer
}

void ThermalNetwork::network_edges_clear()
{
    network_edges.clear();
    network_edges.shrink_to_fit();
}

void ThermalNetwork::network_clear()
{
    network_edges_clear();
    network_nodes_clear();
    next_node_id = 0;
    network_label = "Unnamed network";
}

std::vector<ThermalNode> ThermalNetwork::connected_nodes(size_t id)
{
    std::vector<ThermalNode> result;
    for (ThermalEdge edge : network_edges)
    {
        if (edge.hasNode(id))
        {
            result.push_back((edge.id_0 == id ? network_nodes[edge.id_1] : network_nodes[edge.id_0]));
        }
    }
    return result;
}