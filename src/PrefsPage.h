#pragma once
#include <wx/wx.h>
#include <wx/preferences.h>
#include <wx/spinctrl.h>
#include <wx/valnum.h>
#include <wx/config.h>
#include <wx/choice.h>

// UI panel
class GeneralPrefsPanel : public wxPanel {
public:
    GeneralPrefsPanel(wxWindow* parent);
    
    virtual bool TransferDataToWindow() override;
    virtual bool TransferDataFromWindow() override;

private:
    bool m_is_loading = false;

    wxCheckBox* m_autosave_enable;
    wxSpinCtrl* m_autosave_mins;
    wxSpinCtrl* m_node_size;
    wxChoice* m_scheme;
    wxChoice* m_grid_snapping;
    wxChoice* m_export;
    wxTextCtrl* m_default_dt;
    wxTextCtrl* m_ss_iterations_max;
    wxTextCtrl* m_ss_relaxation;
    wxTextCtrl* m_ss_tolerance_max;
    wxSpinCtrlDouble* m_default_ambient;

    // bind events so Mac applies changes instantly
    void OnPrefChanged(wxCommandEvent& event);
    void OnSpinPrefChanged(wxSpinEvent& event);
    void OnSpinDoublePrefChanged(wxSpinDoubleEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

// Page wrapper
class GeneralPrefsPage : public wxStockPreferencesPage {
public:
    // Kind_General automatically gives page the standard cog
    GeneralPrefsPage() : wxStockPreferencesPage(Kind_General) {}
    
    virtual wxWindow* CreateWindow(wxWindow* parent) override {
        return new GeneralPrefsPanel(parent);
    }
};