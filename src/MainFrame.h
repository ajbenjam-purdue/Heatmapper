#pragma once
#include <wx/wx.h>
#include "ThermalNetwork.h"

class ThermalCanvas; // Fwd declaration

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    void ShowNodeProperties(int node_index);

private:

    wxStaticText* m_node_label;
    wxTextCtrl* m_temp_input;
    wxTextCtrl* m_load_input;
    wxButton* m_apply_button;

    int m_currently_editing_node = -1;

    ThermalNetwork m_active_network;
    ThermalCanvas* m_canvas;

    void OnOpen(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnRunSteadyState(wxCommandEvent& event);
    void OnRunTransient(wxCommandEvent& event);
    void OnApplyProperties(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};