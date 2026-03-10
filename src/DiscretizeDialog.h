#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "MaterialLibrary.h"
#include "EdgeConfigDialog.h"

class DiscretizeDialog : public wxDialog {
public:
    DiscretizeDialog(wxWindow* parent, const MaterialLibrary& mat_lib);

    int GetDiscretizationType() const;
    int GetN() const;
    int GetM() const;
    
    // New Getters for the resistances
    double GetR1() const { return m_res_1; }
    double GetR2() const { return m_res_2; }
    double GetInternalResistance() const;

private:
    const MaterialLibrary& m_mat_lib; 
    wxChoice* m_type_choice;
    wxStaticBitmap* m_svg_display;
    wxBoxSizer* m_dynamic_input_sizer;

    std::unordered_map<std::string, wxTextCtrl*> m_inputs;

    // Resistance State Tracking
    double m_res_1 = 1.0;
    double m_res_2 = 1.0;
    wxTextCtrl* m_r1_display = nullptr;
    wxTextCtrl* m_r2_display = nullptr;

    void OnTypeChange(wxCommandEvent& event);
    void BuildInputs(int selection);
    
    wxTextCtrl* AddInputRow(const wxString& label, const std::string& key, const wxString& default_val = "2");
    
    // New Helper and Handlers
    void AddResistanceConfigRow(const wxString& label, int button_id, wxTextCtrl** out_display, double current_val);
    void OnConfigR1(wxCommandEvent& event);
    void OnConfigR2(wxCommandEvent& event);

    double GetVal(const std::string& key) const;
    int GetIntVal(const std::string& key) const;

    wxDECLARE_EVENT_TABLE();
};