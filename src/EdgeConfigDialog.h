#pragma once
#include <wx/wx.h>
#include <vector>
#include <functional>
#include <string>
#include "MaterialLibrary.h"
#include "ThermalEdge.h"

// For SVG location
wxString resDir = wxStandardPaths::Get().GetResourcesDir();
wxString sep = wxFileName::GetPathSeparator();
wxString svg_path = resDir + sep + "assets" + sep + "resistances" + sep;

// Converted from owned to passed
double GetVal(std::unordered_map<std::string, wxTextCtrl*> inputs, const std::string& key)
{
    auto it = inputs.find(key);
    if (it == inputs.end()) return 0.0;

    double v = 0.0;
    it->second->GetValue().ToDouble(&v);
    return v;
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
};

std::vector<ConfigType> ConfigTypes = {
    ConfigType{
        "Conduction: Rectangular Cross Section",  // name
        CONDUCTION, "conduction_rectangular.svg", // conduction, needs k menu, path
        [](std::unordered_map<std::string, wxTextCtrl*> inputs){return "";}, // Always valid
        [](std::unordered_map<std::string, wxTextCtrl*> inputs){
            double L = GetVal(inputs, "L");
            double k = GetVal(inputs, "k");
            double W = GetVal(inputs, "W");
            double H = GetVal(inputs, "H");
            return PureResistance{L / std::max(k * W * H, 1e-10)};
        },
        {{"Length (L) [m]", "L"}, {"Width (W) [m]", "W"}, {"Height (H) [m]", "H"}, {"Thermal Cond. (k) [W/m-K]", "k"}}
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
    double GetVal(const std::string& key);
private:
    wxDECLARE_EVENT_TABLE();
};