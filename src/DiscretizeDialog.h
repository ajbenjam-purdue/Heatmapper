#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "MaterialLibrary.h"

class DiscretizeDialog : public wxDialog {
public:
    DiscretizeDialog(wxWindow* parent, const MaterialLibrary& mat_lib);

    // Getters for MainFrame to read
    // All defined as member functs for read-only

    int GetDiscretizationType() const; 
    int GetN() const;
    int GetM() const;
    double GetInternalResistance() const;

private:
    const MaterialLibrary& m_mat_lib; // Not yet used
    wxChoice* m_type_choice;
    wxStaticBitmap* m_svg_display;
    wxBoxSizer* m_dynamic_input_sizer;

    std::unordered_map<std::string, wxTextCtrl*> m_inputs;

    void OnTypeChange(wxCommandEvent& event);
    void BuildInputs(int selection);
    
    wxTextCtrl* AddInputRow(const wxString& label, const std::string& key, const wxString& default_val = "2");
    double GetVal(const std::string& key) const;
    int GetIntVal(const std::string& key) const;

    wxDECLARE_EVENT_TABLE();
};