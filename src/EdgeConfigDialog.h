#pragma once
#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <vector>
#include <functional>
#include <string>
#include <unordered_map>
#include "MaterialLibrary.h"
#include "ThermalEdge.h"

// For SVG location
inline wxString e_resDir = wxStandardPaths::Get().GetResourcesDir();
inline wxString e_sep = wxFileName::GetPathSeparator();
inline wxString e_svg_path = e_resDir + e_sep + "assets" + e_sep + "resistances" + e_sep;

namespace EdgeCFG
{
    // Converted from owned to passed
    inline double GetVal(const std::unordered_map<std::string, wxTextCtrl*>& inputs, const std::string& key)
    {
        auto it = inputs.find(key);
        if (it == inputs.end()) return 0.0;

        double v = 0.0;
        it->second->GetValue().ToDouble(&v);
        return v;
    }
}

// Three (+1) types of thermal resistance
enum ResistanceGroups {
    CONDUCTION,
    CONVECTION,
    RADIATION,
    CONTACT
};

// Config container to hold all relevant data
struct ConfigType {

    // Display name
    std::string label;

    // Conduction/Convection/Radiation
    enum ResistanceGroups resistance_type;

    // Filename
    std::string image_name;

    // Validity lambda
    // Returns the message OR nothing at all for no error
    std::function<std::string(std::unordered_map<std::string, wxTextCtrl*> inputs)> is_valid;

    // Execution lambda
    std::function<EdgeParams(std::unordered_map<std::string, wxTextCtrl*> inputs)> get_params;

    // What properties to collect and how to name them
    // {{"Area (A) [m2]:", "A"}, ...}
    std::vector<std::pair<std::string, std::string>> needed_properties;

    // Probably a bad way to do it but these lists are ~4 items long. Returns `true` if the `ConfigType` has `k` as a property and `false` if not.
    bool has_property_k () 
    {
        for (auto [prop_name, prop_label] : needed_properties)
        {
            if (prop_label == "k")
                return true;
        }
        return false;
    }

    wxString get_svg_path()
    {
        return e_svg_path + image_name;
    }
};

inline std::vector<ConfigType> ConfigTypes = {
    ConfigType{ // Planar conduction (rect)
        "Conduction: Rectangular Cross Section",  
        CONDUCTION, "conduction_rectangular.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double L = EdgeCFG::GetVal(inputs, "L"); double k = EdgeCFG::GetVal(inputs, "k");
            double W = EdgeCFG::GetVal(inputs, "W"); double H = EdgeCFG::GetVal(inputs, "H");
            return PureResistance{L / std::max(k * W * H, 1e-10)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Width (W) [m]", "W"}, 
            {"Height (H) [m]", "H"}, 
            {"Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Planar conduction (circ)
        "Conduction: Circular Cross Section",  
        CONDUCTION, "conduction_circular_cross_section.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double L = EdgeCFG::GetVal(inputs, "L"); double D = EdgeCFG::GetVal(inputs, "D");
            double k = EdgeCFG::GetVal(inputs, "k");
            return PureResistance{4.0 * L / std::max(k * M_PI * D * D, 1e-10)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Diameter (D) [m]", "D"}, 
            {"Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Planar conduction (annul)
        "Conduction: Annular Cross Section",  
        CONDUCTION, "conduction_donut_cross_section.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double L = EdgeCFG::GetVal(inputs, "L"); double D1 = EdgeCFG::GetVal(inputs, "D1"); double D2 = EdgeCFG::GetVal(inputs, "D2"); double k = EdgeCFG::GetVal(inputs, "k");
            double D_outer = std::max(D1, D2); double D_inner = std::min(D1, D2);
            return PureResistance{4.0 * L / std::max(k * M_PI * (D_outer * D_outer - D_inner * D_inner), 1e-10)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Inner Diameter (D1) [m]", "D1"}, 
            {"Outer Diameter (D2) [m]", "D2"}, 
            {"Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Radial conduction (cyl)
        "Conduction: Radial (Cylindrical)",  
        CONDUCTION, "conduction_radial.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double L = EdgeCFG::GetVal(inputs, "L"); double D1 = EdgeCFG::GetVal(inputs, "D1"); double D2 = EdgeCFG::GetVal(inputs, "D2"); double k = EdgeCFG::GetVal(inputs, "k");
            double D_outer = std::max(D1, D2); double D_inner = std::min(D1, D2);
            return PureResistance{std::log(D_outer / D_inner) / std::max(2.0 * M_PI * k * L, 1e-10)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Inner Diameter (D1) [m]", "D1"}, 
            {"Outer Diameter (D2) [m]", "D2"}, 
            {"Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Radial conduction (sphere)
        "Conduction: Radial (Spherical)",  
        CONDUCTION, "conduction_spherical.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double D1 = EdgeCFG::GetVal(inputs, "D1"); double D2 = EdgeCFG::GetVal(inputs, "D2"); double k = EdgeCFG::GetVal(inputs, "k");
            double D_outer = std::max(D1, D2); double D_inner = std::min(D1, D2);
            return PureResistance{(1.0 / D_inner - 1.0 / D_outer) / std::max(2.0 * M_PI * k, 1e-10)};
        },
        {
            {"Inner Diameter (D1) [m]", "D1"}, 
            {"Outer Diameter (D2) [m]", "D2"}, 
            {"Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Contact Res
        "Conduction: Contact Resistance",  
        CONDUCTION, "conduction_contact_resistance.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double Rpp = EdgeCFG::GetVal(inputs, "Rpp"); double A = EdgeCFG::GetVal(inputs, "A");
            return PureResistance{Rpp / std::max(A, 1e-8)};
        },
        {
            {"Specific Contact Resistance (R'') [m2]", "R''"}, 
            {"Area (A) [m2]", "A"}
        }
    },
    ConfigType{ // Cross Section conduction
        "Conduction: Constant Cross Section (Known Area)",  
        CONDUCTION, "conduction_area.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double L = EdgeCFG::GetVal(inputs, "L"); double A = EdgeCFG::GetVal(inputs, "A");
            double k = EdgeCFG::GetVal(inputs, "k");
            return PureResistance{L / std::max(k * A, 1e-8)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Area (A) [m2]", "A"}, 
            {"Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Shape Factor Buried Cylinder to Surface
        "Conduction: Cylinder in Medium to Surface",  
        CONDUCTION, "conduction_sf_buried_isothermal_cylinder.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double D = EdgeCFG::GetVal(inputs, "D"); double z = EdgeCFG::GetVal(inputs, "z"); 
            double L = EdgeCFG::GetVal(inputs, "L"); double S = (2.0 * M_PI * L) / std::log(4.0 * z / D);
            return PureResistance{1.0 / std::max(S * EdgeCFG::GetVal(inputs, "k"), 1e-8)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Depth (z) [m]", "z"}, 
            {"Diameter (D) [m]", "D"},
            {"Medium Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Shape Factor Buried Sphere to Surface
        "Conduction: Sphere in Medium to Surface",  
        CONDUCTION, "conduction_sf_buried_isothermal_sphere.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double D = EdgeCFG::GetVal(inputs, "D"); double z = EdgeCFG::GetVal(inputs, "z"); 
            double S = (2.0 * M_PI * D) / (1.0 - D / (4.0 * z));
            return PureResistance{1.0 / std::max(S * EdgeCFG::GetVal(inputs, "k"), 1e-8)};
        },
        {
            {"Depth (z) [m]", "z"}, 
            {"Diameter (D) [m]", "D"},
            {"Medium Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Shape Factor Two Cylinders in Medium
        "Conduction: Parallel Cylinders in Medium",  
        CONDUCTION, "conduction_sf_two_cylinders.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double L = EdgeCFG::GetVal(inputs, "L"); double D1 = EdgeCFG::GetVal(inputs, "D1"); 
            double D2 = EdgeCFG::GetVal(inputs, "D2"); double w = EdgeCFG::GetVal(inputs, "w"); 
            double S = 2.0 * M_PI * L / std::acosh((4.0 * w * w - D1 * D1 - D2 * D2) / (2.0 * D1 * D2));
            return PureResistance{1.0 / std::max(S * EdgeCFG::GetVal(inputs, "k"), 1e-8)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Diameter 1 (D1) [m]", "D1"}, 
            {"Diameter 2 (D2) [m]", "D2"},
            {"Medium Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Shape Factor Vertical Cylinder to Surface
        "Conduction: Vertical Cylinder in Medium to Surface",  
        CONDUCTION, "conduction_sf_vertical_cylinder.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            double D = EdgeCFG::GetVal(inputs, "D"); double L = EdgeCFG::GetVal(inputs, "L"); 
            double S = 2.0 * M_PI * L / std::log(4.0 * L / D);
            return PureResistance{1.0 / std::max(S * EdgeCFG::GetVal(inputs, "k"), 1e-8)};
        },
        {
            {"Length (L) [m]", "L"}, 
            {"Cylinder Diameter (D) [m]", "D"}, 
            {"Medium Thermal Cond. (k) [W/m-K]", "k"}
        }
    },
    ConfigType{ // Radiation to Surroundings
        "Radiation: Surface to Large Surroundings",  
        CONDUCTION, "radiation_surroundings.svg", 
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){return "";}, // Always valid
        [](const std::unordered_map<std::string, wxTextCtrl*> &inputs){
            return RadiationUniform{ EdgeCFG::GetVal(inputs, "epsilon"), EdgeCFG::GetVal(inputs, "A") };
        },
        {
            {"Surface Area (A) [m2]", "A"}, 
            {"Emissivity (ε) [-]", "epsilon"}
        }
    }
};

class EdgeConfigDialog : public wxDialog {
public:
    EdgeConfigDialog(wxWindow* parent, const MaterialLibrary& mat_lib);

    // Legacy
    double GetCalculatedResistance() const { return m_calculated_res; }

    // New
    EdgeParams GetEdgeParameters() const { return m_returned_params; }
private:
    const MaterialLibrary& m_mat_lib; // Store the reference

    wxChoice* m_type_choice;
    wxChoice* m_material_choice = nullptr; // New dropdown
    wxStaticBitmap* m_svg_display;
    wxBoxSizer* m_dynamic_input_sizer;
    
    std::unordered_map<std::string, wxTextCtrl*> m_inputs;
    wxTextCtrl* m_k_input = nullptr; // Track the 'k' box specifically
    wxStaticText* m_warning_text; // Warning text for deviations from correlation
    
    double m_calculated_res = 0.0;
    EdgeParams m_returned_params;

    void OnTypeChange(wxCommandEvent& event);
    void OnMaterialChange(wxCommandEvent& event); // New event handler
    void OnOK(wxCommandEvent& event);
    
    void BuildInputs(int selection);
    void ValidateInputs();
    void OnInputChanged(wxCommandEvent& event);
    wxTextCtrl* AddInputRow(const wxString& label, const std::string& key);

    // No longer used
    // double EdgeCFG::GetVal(const std::string& key);
private:
    wxDECLARE_EVENT_TABLE();
};