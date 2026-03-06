#pragma once
#include <wx/wx.h>
#include <unordered_set>
#include "ThermalNetwork.h"
#include "MainFrame.h"

// Toolbar enum
enum class ToolMode {
    SELECT,
    ADD_NODE,
    ADD_EDGE,
    DELETE_ITEM
};

class ThermalCanvas : public wxPanel {
public:
    const static int CANVAS_MARGIN = 20; // pixels, on each side
    const static int EDGE_SELECTION_TOLERANCE = 12; // pixels away for valid hit
    const static float constexpr NODE_RADIUS = 10.0; // pixels
    const static float constexpr SNAP_MARGIN = 8; // pixels

    const wxColour COLOR_UNKNOWN = wxColour(120, 120, 140); // Color for nodes with unknown values
    const wxColour COLOR_SELECT = wxColour(215, 200, 0); // Color for actively selected node/edge
    const wxColour COLOR_DESELECT = wxColour(150, 150, 150);
    const wxColour COLOR_GUIDE = wxColour(90, 90, 95); // Color for guidelines
    const wxColour COLOR_EDGE = wxColour(90, 90, 95); // Color for edges
    const wxColour COLOR_EDGE_BACKDROP = wxColour(0, 0, 0, 75); // Color for edges (shadow)

    // State
    std::unordered_set<int> m_sel_node_ids;
    //int m_sel_node_id = -1; Conversion for shift click
    int m_sel_edge_index = -1;
    bool m_is_dragging = false;
    bool m_is_box_selecting = false;
    wxPoint m_box_start;
    wxPoint m_current_mouse_pos;
    wxPoint m_drag_start_mouse; 
    std::unordered_map<int, std::pair<double, double>> m_drag_start_nodes;

    //Snapping
    double SNAP_X = -1.0; // Init and held at -1 unless real coords exist. 
    double SNAP_Y = -1.0; // Relative (0-1)

    // Interaction
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

    ThermalCanvas(wxWindow* parent);
    
    // Allow the MainFrame to give the canvas a network to draw
    void SetNetwork(ThermalNetwork* network);

    // Toolbar
    ToolMode m_current_tool = ToolMode::SELECT;
    int m_first_edge_node = -1; // Memory for the edge tool

    void SetToolMode(ToolMode mode);

private:
    ThermalNetwork* m_network = nullptr; // Pointer to the data

    void OnPaint(wxPaintEvent& event);
    
    wxDECLARE_EVENT_TABLE(); 
};