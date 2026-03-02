#include "ThermalCanvas.h"
#include <wx/graphics.h>

wxBEGIN_EVENT_TABLE(ThermalCanvas, wxPanel)
    EVT_PAINT(ThermalCanvas::OnPaint)
wxEND_EVENT_TABLE()

ThermalCanvas::ThermalCanvas(wxWindow* parent) 
    : wxPanel(parent, wxID_ANY) {
    SetBackgroundColour(wxColour(40, 44, 52));
}

void ThermalCanvas::SetNetwork(ThermalNetwork* network) {
    m_network = network;
    Refresh(); // wipe the canvas and trigger OnPaint immediately
}

void ThermalCanvas::OnPaint(wxPaintEvent& event) {
    // Create the base painter & upgrade to a high-quality graphics context for smooth rendering
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    
    // If the upgrade fails or we have no network, abort drawing safely
    if (!gc || !m_network) return;

    // Get the current window size so we can scale the 0.0-1.0 coordinates
    int width, height;
    GetClientSize(&width, &height);

    // Draw edges, then nodes
    gc->SetPen(wxPen(wxColour(150, 150, 150), 3)); // 3 pt 150 gray

    for (const ThermalEdge& edge : m_network->network_edges) {
        // Find the two nodes this edge connects
        const ThermalNode& n0 = m_network->network_nodes[edge.id_0];
        const ThermalNode& n1 = m_network->network_nodes[edge.id_1];

        double x1 = n0.canvas_position_x * width;
        double y1 = n0.canvas_position_y * height;
        double x2 = n1.canvas_position_x * width;
        double y2 = n1.canvas_position_y * height;

        gc->StrokeLine(x1, y1, x2, y2);
    }

    // Now draw nodes
    gc->SetPen(wxPen(wxColour(200, 200, 200), 2)); // Node outline
    gc->SetBrush(wxBrush(wxColour(50, 120, 200))); // Node fill color (Blue)

    double node_radius = 12.0;

    for (const ThermalNode& node : m_network->network_nodes) {
        double x = (node.canvas_position_x * width) - node_radius;
        double y = (node.canvas_position_y * height) - node_radius;

        gc->DrawEllipse(x, y, node_radius * 2, node_radius * 2);
    }
}