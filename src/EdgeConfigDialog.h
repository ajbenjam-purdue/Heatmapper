#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include "MaterialLibrary.h"

class EdgeConfigDialog : public wxDialog {
public:
    EdgeConfigDialog(wxWindow* parent, const MaterialLibrary& mat_lib);
    double GetCalculatedResistance() const { return m_calculated_res; }

private:
    const MaterialLibrary& m_mat_lib; // Store the reference

    wxChoice* m_type_choice;
    wxChoice* m_material_choice = nullptr; // New dropdown
    wxStaticBitmap* m_svg_display;
    wxBoxSizer* m_dynamic_input_sizer;
    
    std::unordered_map<std::string, wxTextCtrl*> m_inputs;
    wxTextCtrl* m_k_input = nullptr; // Track the 'k' box specifically
    wxStaticText* m_warning_text; // Warnng text for deviations from correlation
    
    double m_calculated_res = 0.0;

    void OnTypeChange(wxCommandEvent& event);
    void OnMaterialChange(wxCommandEvent& event); // New event handler
    void OnOK(wxCommandEvent& event);
    
    void BuildInputs(int selection);
    void ValidateInputs();
    void OnInputChanged(wxCommandEvent& event);
    wxTextCtrl* AddInputRow(const wxString& label, const std::string& key);
    double GetVal(const std::string& key);

    wxDECLARE_EVENT_TABLE();
};