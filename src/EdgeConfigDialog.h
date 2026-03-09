#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>

class EdgeConfigDialog : public wxDialog {
public:
    EdgeConfigDialog(wxWindow* parent);
    double GetCalculatedResistance() const { return m_calculated_res; }

private:
    wxChoice* m_type_choice;
    wxStaticBitmap* m_svg_display;
    wxBoxSizer* m_dynamic_input_sizer;
    
    // Store pointers to dynamic text boxes so they can by read later
    std::vector<wxTextCtrl*> m_current_inputs; 
    double m_calculated_res = 0.0;

    void OnTypeChange(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    
    // Helper to rebuild the UI
    void BuildInputs(int selection);

    wxDECLARE_EVENT_TABLE();
};