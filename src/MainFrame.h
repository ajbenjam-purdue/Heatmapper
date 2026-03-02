#pragma once
#include <wx/wx.h>
#include "ThermalNetwork.h"
#include "ThermalCanvas.h"

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:
    ThermalNetwork m_active_network;
    ThermalCanvas* m_canvas;

    void OnOpen(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};