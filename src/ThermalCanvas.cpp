#include "ThermalCanvas.h"
#include "utils.h"
#include <wx/graphics.h>

wxBEGIN_EVENT_TABLE(ThermalCanvas, wxPanel)
    EVT_PAINT(ThermalCanvas::OnPaint)
        EVT_LEFT_DOWN(ThermalCanvas::OnMouseLeftDown)
            EVT_LEFT_UP(ThermalCanvas::OnMouseLeftUp)
                EVT_MOTION(ThermalCanvas::OnMouseMove)
                    wxEND_EVENT_TABLE()

                        ThermalCanvas::ThermalCanvas(wxWindow *parent)
    : wxPanel(parent, wxID_ANY)
{
    SetBackgroundColour(wxColour(40, 44, 52));
}

void ThermalCanvas::SetNetwork(ThermalNetwork *network)
{
    m_network = network;
    Refresh(); // wipe the canvas and trigger OnPaint immediately
}

void ThermalCanvas::OnPaint(wxPaintEvent &event)
{
    // Create the base painter & upgrade to a high-quality graphics context for smooth rendering
    wxPaintDC dc(this);
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

    // If the upgrade fails or we have no network, abort drawing safely
    if (!gc || !m_network)
        return;

    // Get the current window size so we can scale the 0.0-1.0 coordinates
    int width, height;
    GetClientSize(&width, &height);

    // Draw edges, then nodes
    gc->SetPen(wxPen(wxColour(150, 150, 150), 3)); // 3 pt 150 gray

    for (const ThermalEdge &edge : m_network->network_edges)
    {
        // Find the two nodes this edge connects
        const ThermalNode &n0 = m_network->network_nodes[edge.id_0];
        const ThermalNode &n1 = m_network->network_nodes[edge.id_1];

        double x1 = n0.canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double y1 = n0.canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double x2 = n1.canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double y2 = n1.canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

        gc->StrokeLine(x1, y1, x2, y2);
    }

    // Now draw nodes
    gc->SetBrush(wxBrush(wxColour(50, 120, 200))); // Node fill color (Blue)

    double max_node_temperature = m_network->highest_node_temperature();
    double min_node_temperature = m_network->lowest_node_temperature();

    // Text
    gc->SetFont(wxFontInfo(10).Family(wxFONTFAMILY_SWISS), wxColour(255, 255, 255));

    // NODE LOOP
    for (const ThermalNode &node : m_network->network_nodes)
    {

        // Node position
        double x = node.canvas_position_x * (width - CANVAS_MARGIN * 2) - NODE_RADIUS + CANVAS_MARGIN;
        double y = node.canvas_position_y * (height - CANVAS_MARGIN * 2) - NODE_RADIUS + CANVAS_MARGIN;

        // Heat flux out/in
        if (node.ext_load > 0.0)
        {
            gc->SetPen(wxPen(wxColour(255, 120, 0), 3)); // 3px Orange Pen

            // Find the center of the node
            double center_x = x + NODE_RADIUS;
            double center_y = y + NODE_RADIUS;

            // The tip of the arrow rests just outside the top edge of the circle
            double tip_x = center_x;
            double tip_y = center_y - NODE_RADIUS - 2;

            // Draw the vertical shaft (15 pixels long)
            gc->StrokeLine(tip_x, tip_y - 15, tip_x, tip_y);

            // Draw the left and right arrow heads
            gc->StrokeLine(tip_x, tip_y, tip_x - 5, tip_y - 5);
            gc->StrokeLine(tip_x, tip_y, tip_x + 5, tip_y - 5);
        }

        // Node color
        if (max_node_temperature == min_node_temperature)
        {
            gc->SetBrush(wxBrush(wxColour(50, 120, 200)));
            gc->SetPen(wxPen(wxColour(30, 100, 180)));
        }
        else
        {
            double rat = (node.node_temperature - min_node_temperature) / (max_node_temperature - min_node_temperature);
            gc->SetBrush(wxBrush(wxColour((int)(200 * rat) + 30, 35, int(200 * (1 - rat)) + 30)));
            gc->SetPen(wxPen(wxColour((int)(200 * rat) + 10, 15, int(200 * (1 - rat)) + 10)));
        }

        // Selection pen
        if (node.node_id == m_sel_node_index)
        {
            gc->SetPen(wxPen(wxColour(255, 255, 0), 4)); // Thick yellow pen
        }

        gc->DrawEllipse(x, y, NODE_RADIUS * 2, NODE_RADIUS * 2);

        // Format the temperature to 1 decimal place
        wxString temp_text = wxString::Format("%.1f °C", node.node_temperature);

        // Calculate text position (shift right by the diameter + a 5px margin)
        double text_x = x + (NODE_RADIUS * 2) + 5;
        double text_y = y + (NODE_RADIUS / 2); // Center it vertically a bit

        // Draw it!
        gc->DrawText(temp_text, text_x, text_y);
    }
}

void ThermalCanvas::OnMouseLeftDown(wxMouseEvent &event)
{
    if (!m_network)
        return;

    // Get window size to scale the normalized coordinates
    int width, height;
    GetClientSize(&width, &height);

    // Get the mouse click coordinates
    wxPoint mouse_pos = event.GetPosition();

    // Reset selections
    m_sel_node_index = -1;
    m_sel_edge_index = -1; 

    for (size_t i = 0; i < m_network->network_nodes.size(); i++)
    {
        const ThermalNode &node = m_network->network_nodes[i];

        // Calculate screen coordinates of this node
        double node_x = node.canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double node_y = node.canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

        // distance to node
        double dx = mouse_pos.x - node_x;
        double dy = mouse_pos.y - node_y;
        double distance = std::sqrt((dx * dx) + (dy * dy));

        // Is the click inside the radius?
        if (distance <= NODE_RADIUS)
        {
            m_sel_node_index = i;
            m_is_dragging = true;


            // Update UI
            ((MainFrame *)GetParent())->ShowNodeProperties(i);


            break; // Stop checking, node located
        }
    }

    // Node still not found. Check for edge
    if (m_sel_node_index == -1)
    {
        for (size_t i = 0; i < m_network->network_edges.size(); i++)
        {
            const ThermalEdge &edge = m_network->network_edges[i];

            // Convert everything to screen px
            double edge_x_1 = m_network->network_nodes[edge.id_0].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double edge_y_1 = m_network->network_nodes[edge.id_0].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double edge_x_2 = m_network->network_nodes[edge.id_0].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double edge_y_2 = m_network->network_nodes[edge.id_0].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

            // Check first if bounding box criteria is met
            if (in_bounding_box(mouse_pos.x, mouse_pos.y, edge_x_1, edge_y_1, edge_x_2, edge_y_2) && distance_perpendicular(mouse_pos.x, mouse_pos.y, edge_x_1, edge_y_1, edge_x_2, edge_y_2) <= EDGE_SELECTION_TOLERANCE)
            {
                m_sel_edge_index = i; // Set selected edge

                // Update UI
                ((MainFrame *)GetParent())->ShowEdgeProperties(i);

                break;
            }
        }
    }
    Refresh(); // Redraw to show selection highlight, exit out
}

void ThermalCanvas::OnMouseMove(wxMouseEvent &event)
{
    if (m_is_dragging && m_sel_node_index != -1 && m_network)
    {

        int width, height;
        GetClientSize(&width, &height);

        // Prevent division by zero if window is collapsed
        if (width == 0 || height == 0)
            return;

        wxPoint mouse_pos = event.GetPosition();

        // Convert pixel coordinates back to normalized fractions
        double new_norm_x = ((double)mouse_pos.x - CANVAS_MARGIN) / (width - CANVAS_MARGIN * 2);
        double new_norm_y = ((double)mouse_pos.y - CANVAS_MARGIN) / (height - CANVAS_MARGIN * 2);

        // Clamp values between 0.0 and 1.0 so nodes can't be dragged off-screen
        new_norm_x = std::clamp(new_norm_x, 0.0, 1.0);
        new_norm_y = std::clamp(new_norm_y, 0.0, 1.0);

        // Update the actual network data
        m_network->network_nodes[m_sel_node_index].canvas_position_x = new_norm_x;
        m_network->network_nodes[m_sel_node_index].canvas_position_y = new_norm_y;

        // Redraw to make the node follow the cursor
        Refresh();
    }
}

void ThermalCanvas::OnMouseLeftUp(wxMouseEvent &event)
{
    m_is_dragging = false;
}