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
    // m
    double length;

    // w/m-K
    double k;

    // m2
    double area;
};

struct ConvectionUniform {
    // w/m2-K
    double h;

    // m2
    double area;
};

struct RadiationUniform {
    // [-]
    double epsilon;

    // m2
    double area;
};

struct PureResistance {
    // K/W
    double R;
};

using EdgeParams = std::variant<ConductionUniform, ConvectionUniform, RadiationUniform, PureResistance>;

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
    double resistance(double T_1, double T_2) const;
    EdgeParams params;

    bool hasNode(size_t id);
};

std::ostream& operator<<(std::ostream& os, const ThermalEdge& node);

std::ostream& operator<<(std::ostream& os, const EdgeType& type);