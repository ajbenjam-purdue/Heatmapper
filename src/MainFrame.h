#pragma once
#include <wx/wx.h>
#include <wx/artprov.h>
#include "ThermalNetwork.h"

class ThermalCanvas; // Fwd declaration

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    void ShowNodeProperties(int node_id);
    void ResetPropertiesWindow();
    void ShowEdgeProperties(int edge_index);
    void ForceSelectTool();

private:

    wxStaticText* m_node_label; // What the node is labeled
    wxTextCtrl* m_temp_input; // What the temp should be
    wxCheckBox* m_is_fixed_checkbox; // If the temp is a fixed point
    wxTextCtrl* m_load_input; // What the input load is (W)

    wxTextCtrl* m_res_input; // What the edge resistance should be

    wxStaticText* m_flow_disp_label; // Label for edge's measured heat flux
    wxTextCtrl* m_flow_disp; // Actual number

    wxButton* m_apply_button; // Apply button

    // Labels
    wxStaticText* m_temp_label;
    wxStaticText* m_load_label;
    wxStaticText* m_thermal_res_label;

    int m_currently_editing_node = -1;
    int m_currently_editing_edge = -1;

    ThermalNetwork m_active_network;
    ThermalCanvas* m_canvas;

    void OnOpen(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnRunSteadyState(wxCommandEvent& event);
    void OnRunTransient(wxCommandEvent& event);
    void OnApplyProperties(wxCommandEvent& event);
    void OnToolSelect(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};