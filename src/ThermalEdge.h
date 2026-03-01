#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <cstddef>

enum class EdgeType {
    CONDUCTION_UNIFORM,
    CONVECTION_UNIFORM,
    RADIATION_UNIFORM,
    RESISTANCE_PURE
};

struct ConductionUniform {
    double length;
    double k;
    double area;
};

struct ConvectionUniform {
    double h;
    double area;
};

// NOT DONE
struct RadiationUniform {
    double h;
    double area;
};

struct PureResistance {
    double R; // K/W
};

class ThermalEdge {
    public:
    size_t id_0, id_1;
    EdgeType type;
    double flux;

    ThermalEdge(size_t id_a, size_t id_b, ConductionUniform p);
    ThermalEdge(size_t id_a, size_t id_b, ConvectionUniform p);
    ThermalEdge(size_t id_a, size_t id_b, RadiationUniform p);
    ThermalEdge(size_t id_a, size_t id_b, PureResistance p);

    double resistance() const;
    std::variant<ConductionUniform, ConvectionUniform, RadiationUniform, PureResistance> params;
};

std::ostream& operator<<(std::ostream& os, const ThermalEdge& node);

std::ostream& operator<<(std::ostream& os, const EdgeType& type);