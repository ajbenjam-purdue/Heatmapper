#pragma once
#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/bmpbndl.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/preferences.h>
#include <wx/config.h>
#include <wx/valnum.h>
#include <format>
#include <fstream>
#include <thread>
#include "ThermalNetwork.h"
#include "MaterialLibrary.h"
#include "MaterialDialog.h"
#include "ThermalCanvas.h"
#include "ThermalSolver.h"
#include "EdgeConfigDialog.h"
#include "DiscretizeDialog.h"
#include "TransientDialog.h"
#include "json.hpp"
#include "PrefsPage.h"
#include "utils.h"

class ThermalCanvas; // Fwd declaration

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    
    MaterialLibrary m_materials;

    void ShowNodeProperties(int node_id);
    void ResetPropertiesWindow();
    void ShowEdgeProperties(int edge_index);
    void ForceSelectTool();
    void UpdateToolbarIcons();
    void UpdateDynamicMenus();
    void OnPreferences(wxCommandEvent& evt);

private:

    wxStaticText* m_node_label; // What the node is labeled (label)
    wxTextCtrl* m_node_label_str; // What the node is labeled (actual value)
    wxTextCtrl* m_temp_input; // What the temp should be
    wxCheckBox* m_is_fixed_checkbox; // If the temp is a fixed point
    wxTextCtrl* m_load_input; // What the input load is (W)

    wxTextCtrl* m_res_input; // What the edge resistance should be

    wxStaticText* m_flow_disp_label; // Label for edge's measured heat flux
    wxTextCtrl* m_flow_disp; // Actual number
    wxButton* m_edge_config_button; // Open configuration window button

    wxButton* m_apply_button; // Apply button

    // Labels
    wxStaticText* m_temp_label;
    wxStaticText* m_load_label;
    wxStaticText* m_thermal_res_label;

    // Path
    wxString matFilePath;

    int m_currently_editing_node = -1;
    int m_currently_editing_edge = -1;

    ThermalNetwork m_active_network;
    ThermalCanvas* m_canvas;

    wxPreferencesEditor m_prefs_editor;

    // Node configuration
    wxStaticText* m_mat_label;
    wxChoice* m_mat_choice;
    wxStaticText* m_mass_label;
    wxTextCtrl* m_mass_input;
    wxStaticText* m_cp_label;
    wxTextCtrl* m_cp_input;

    // Node/Edge property methods
    void ApplyCurrentProperties();
    
    void OnParameterEnter(wxCommandEvent& event);
    void OnParameterFocusLost(wxFocusEvent& event);
    void OnMaterialSelected(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnRunSteadyState(wxCommandEvent& event);
    void OnRunTransient(wxCommandEvent& event);
    // void OnApplyProperties(wxCommandEvent& event);
    void OnToolSelect(wxCommandEvent& event);
    void OnCharHook(wxKeyEvent& event);
    void OnEdgeConfigButtonClicked(wxCommandEvent& event);
    void OnDiscretizeButtonClicked(wxCommandEvent& event);
    void OnMaterialLibOpened(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};