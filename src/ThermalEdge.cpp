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

// Legacy function, since who knows where all the calls are
double ThermalEdge::resistance() const {
    return std::visit([](auto&& p) -> double {
        using T = std::decay_t<decltype(p)>;

        if constexpr (std::is_same_v<T, PureResistance>) {
            return p.R;
        }
        else {
            return 1e10;
        }
    }, params);
}

// New function
double ThermalEdge::resistance(double T1, double T2) const {
    return std::visit([T1, T2](auto&& p) -> double {
        using T = std::decay_t<decltype(p)>;

        if constexpr (std::is_same_v<T, ConductionUniform>) {
            return p.length / (p.k * p.area);
        }
        else if constexpr (std::is_same_v<T, ConvectionUniform>) {
            return 1.0 / (p.h * p.area);
        }
        else if constexpr (std::is_same_v<T, RadiationUniform>) {
            // Celsius to Kelvin (Radiation only)
            double t1_k = T1 + 273.15;
            double t2_k = T2 + 273.15;
            
            // Prevent div-by-zero for un-inst temperature nodes
            if (std::abs(t1_k - t2_k) < 1e-6) t1_k += 1e-6; 

            // Linearization
            const double sigma = 5.670374419e-8;
            double hr = p.epsilon * sigma * (t1_k + t2_k) * (t1_k * t1_k + t2_k * t2_k);
            
            // Eff. resistance is just 1/(h_r*A)
            return 1.0 / std::max(hr * p.area, 1e-10);
        }
        else if constexpr (std::is_same_v<T, PureResistance>) {
            return p.R; // Ignores T1 and T2 entirely
        }
        else {
            return 1e10; // Fallback
        }
    }, params);
}

std::ostream& operator<<(std::ostream& os, const ThermalEdge& edge) {
    os << "Edge between node " << edge.id_0 << " and node " << edge.id_1 << " (" << edge.type << ")";
    return os;
}

bool ThermalEdge::hasNode(size_t id)
{
    return id_0 == id || id_1 == id;
}