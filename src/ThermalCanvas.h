#pragma once
#include <wx/wx.h>
#include "ThermalNetwork.h"
#include "MainFrame.h"

class ThermalCanvas : public wxPanel {
public:
    const static int CANVAS_MARGIN = 20; // pixels, on each side
    const static int EDGE_SELECTION_TOLERANCE = 12; // pixels away for valid hit
    const static float constexpr NODE_RADIUS = 10.0; // pixels

    // State
    int m_sel_node_index = -1;
    int m_sel_edge_index = -1;
    bool m_is_dragging = false;

    // Interaction
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

    ThermalCanvas(wxWindow* parent);
    
    // Allow the MainFrame to give the canvas a network to draw
    void SetNetwork(ThermalNetwork* network);

private:
    ThermalNetwork* m_network = nullptr; // Pointer to the data

    void OnPaint(wxPaintEvent& event);
    
    wxDECLARE_EVENT_TABLE(); 
};