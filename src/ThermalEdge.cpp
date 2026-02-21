#include "ThermalEdge.h"
#include <type_traits>

std::ostream& operator<<(std::ostream& os, const EdgeType& type) {
    switch (type) {
        case EdgeType::CONDUCTION_UNIFORM:
            os << "Uniform Conduction";
            break;
        case EdgeType::CONVECTION_UNIFORM:
            os << "Uniform Convection";
            break;
        case EdgeType::RADIATION_UNIFORM:
            os << "Uniform Radiation";
            break;
        case EdgeType::RESISTANCE_PURE:
            os << "Pure Resistance";
            break;
        default:
            os << "Unknown";
            break;
    }
    return os;
}

ThermalEdge::ThermalEdge(size_t id_a, size_t id_b, ConductionUniform p)
    : id_0(id_a), id_1(id_b),
        type(EdgeType::CONDUCTION_UNIFORM),
        params(p) {}

ThermalEdge::ThermalEdge(size_t id_a, size_t id_b, ConvectionUniform p)
    : id_0(id_a), id_1(id_b),
        type(EdgeType::CONVECTION_UNIFORM),
        params(p) {}

ThermalEdge::ThermalEdge(size_t id_a, size_t id_b, RadiationUniform p)
    : id_0(id_a), id_1(id_b),
        type(EdgeType::RADIATION_UNIFORM),
        params(p) {}

ThermalEdge::ThermalEdge(size_t id_a, size_t id_b, PureResistance p)
    : id_0(id_a), id_1(id_b),
        type(EdgeType::RESISTANCE_PURE),
        params(p) {}

double ThermalEdge::resistance() const {
    return std::visit([](auto&& p) -> double {
        using T = std::decay_t<decltype(p)>;

        if constexpr (std::is_same_v<T, ConductionUniform>) {
            return p.length / (p.k * p.area);
        }
        else if constexpr (std::is_same_v<T, ConvectionUniform>) {
            return 1.0 / (p.h * p.area);
        }
        else if constexpr (std::is_same_v<T, PureResistance>) {
            return p.R;
        }
        else {
            return 0.0;
        }
    }, params);
}

std::ostream& operator<<(std::ostream& os, const ThermalEdge& edge) {
    os << "Edge between node " << edge.id_0 << " and node " << edge.id_0 << " (" << edge.type << ")";
    return os;
}