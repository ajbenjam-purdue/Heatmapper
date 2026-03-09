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
    
    std::vector<wxTextCtrl*> m_current_inputs; 
    wxTextCtrl* m_k_input = nullptr; // Track the 'k' box specifically!
    
    double m_calculated_res = 0.0;

    void OnTypeChange(wxCommandEvent& event);
    void OnMaterialChange(wxCommandEvent& event); // New event handler
    void OnOK(wxCommandEvent& event);
    
    void BuildInputs(int selection);

    wxDECLARE_EVENT_TABLE();
};