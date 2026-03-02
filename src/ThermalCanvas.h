#pragma once
#include <wx/wx.h>
#include "ThermalNetwork.h"

class ThermalCanvas : public wxPanel {
public:
    ThermalCanvas(wxWindow* parent);
    
    // Allow the MainFrame to give the canvas a network to draw
    void SetNetwork(ThermalNetwork* network);

private:
    ThermalNetwork* m_network = nullptr; // Pointer to the data

    void OnPaint(wxPaintEvent& event);
    
    wxDECLARE_EVENT_TABLE(); 
};