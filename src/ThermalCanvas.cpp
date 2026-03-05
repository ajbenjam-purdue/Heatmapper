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

    // Draw snapping guides
    gc->SetPen(wxPen(COLOR_GUIDE, 2));

    if (SNAP_X != -1.0)
    {
        double guideline_x = SNAP_X * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        gc->StrokeLine(guideline_x, 0, guideline_x, height);
    }
    if (SNAP_Y != -1.0)
    {
        double guideline_y = SNAP_Y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        gc->StrokeLine(0, guideline_y, width, guideline_y);
    }

    // Draw edges, then nodes
    gc->SetPen(wxPen(wxColour(150, 150, 150), 3));

    for (size_t i = 0; i < m_network->network_edges.size(); i++)
    {
        // Find the two nodes this edge connects
        const ThermalEdge &edge = m_network->network_edges[i];
        const ThermalNode &n0 = m_network->network_nodes[edge.id_0];
        const ThermalNode &n1 = m_network->network_nodes[edge.id_1];

        double x1 = n0.canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double y1 = n0.canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double x2 = n1.canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double y2 = n1.canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

        if (i == m_sel_edge_index)
        {
            gc->SetPen(wxPen(COLOR_SELECT, 4)); // Thick yellow pen
        }
        else
            gc->SetPen(wxPen(COLOR_DESELECT, 2));

        gc->StrokeLine(x1, y1, x2, y2);
    }

    // Now draw nodes
    gc->SetBrush(wxBrush(COLOR_UNKNOWN)); // Unknown state

    double max_node_temperature = m_network->highest_node_temperature();
    double min_node_temperature = m_network->lowest_node_temperature();

    // Text
    gc->SetFont(wxFontInfo(10).Family(wxFONTFAMILY_SWISS), wxColour(255, 255, 255));

    // NODE LOOP
    for (const auto &[id, node] : m_network->network_nodes)
    {

        // Node position
        double x = node.canvas_position_x * (width - CANVAS_MARGIN * 2) - NODE_RADIUS + CANVAS_MARGIN;
        double y = node.canvas_position_y * (height - CANVAS_MARGIN * 2) - NODE_RADIUS + CANVAS_MARGIN;

        // Heat flux out/in
        if (std::abs(node.ext_load) > 0.0)
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
            if (node.ext_load > 0.0)
            {
                gc->StrokeLine(tip_x, tip_y, tip_x - 5, tip_y - 5);
                gc->StrokeLine(tip_x, tip_y, tip_x + 5, tip_y - 5);
            }
            else
            {
                gc->StrokeLine(tip_x, tip_y - 15, tip_x - 5, tip_y - 10);
                gc->StrokeLine(tip_x, tip_y - 15, tip_x + 5, tip_y - 10);
            }
        }

        // Node color
        if (std::abs(max_node_temperature-min_node_temperature) < 0.01)
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
        if (node.node_id == m_sel_node_id)
        {
            gc->SetPen(wxPen(COLOR_SELECT, 3)); // Thick yellow pen
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

    // Get the mouse click coordinates (And normalize them)
    wxPoint mouse_pos = event.GetPosition();
    double norm_x = ((double)mouse_pos.x - CANVAS_MARGIN) / (width - CANVAS_MARGIN * 2);
    double norm_y = ((double)mouse_pos.y - CANVAS_MARGIN) / (height - CANVAS_MARGIN * 2);
    norm_x = std::clamp(norm_x, 0.0, 1.0);
    norm_y = std::clamp(norm_y, 0.0, 1.0);

    // Reset selections
    m_sel_node_id = -1;
    m_sel_edge_index = -1;

    // Switch for tools
    switch (m_current_tool)
    {
    case ToolMode::SELECT:
    {
        for (auto const &[id, node] : m_network->network_nodes)
        {
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
                m_sel_node_id = node.node_id;
                m_is_dragging = true;

                // Update UI
                ((MainFrame *)GetParent())->ShowNodeProperties(node.node_id);

                break; // Stop checking, node located
            }
        }

        // Node still not found. Check for edge
        if (m_sel_node_id == -1)
        {
            for (size_t i = 0; i < m_network->network_edges.size(); i++)
            {
                const ThermalEdge &edge = m_network->network_edges[i];

                // Convert everything to screen px
                double edge_x_1 = m_network->network_nodes[edge.id_0].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
                double edge_y_1 = m_network->network_nodes[edge.id_0].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
                double edge_x_2 = m_network->network_nodes[edge.id_1].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
                double edge_y_2 = m_network->network_nodes[edge.id_1].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

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
        break;
    }
    case ToolMode::ADD_NODE:
    {
        // Create a node with default parameters and add to network
        ThermalNode new_node(norm_x, norm_y, 1.0, 500.0, "New Node", 0, 15.0);
        m_network->add_node(new_node);

        // If user is pressing shift, multi add. Otherwise, edit properties
        if (!event.ShiftDown())
        {
            SetToolMode(ToolMode::SELECT);
            m_sel_node_id = new_node.node_id;
            ((MainFrame *)GetParent())->ShowNodeProperties(new_node.node_id);
        }
        break;
    }
    case ToolMode::ADD_EDGE:
    {
        // Add edge between two nodes

        // Get node hit (if any)
        int clicked_node_id = -1;
        for (const auto &[id, node] : m_network->network_nodes)
        {
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
                clicked_node_id = id;

                break; // Stop checking, node located
            }
        }

        // Good hit
        if (clicked_node_id != -1)
        {
            if (m_first_edge_node == -1) // First instance, remember
            {
                m_first_edge_node = clicked_node_id;
            }
            else if (m_first_edge_node != clicked_node_id) // Already have a first node (valid)
            {
                m_network->add_edge(ThermalEdge(m_first_edge_node, clicked_node_id, PureResistance{10.0}));
                m_first_edge_node = -1;
            }
            else // Have a first node but it's the same. just reset
            {
                m_first_edge_node = -1;
            }
        }
        break;
    }
    case ToolMode::DELETE_ITEM:
    {
        // Delete node

        // Get node hit (if any)
        int clicked_node_id = -1;
        for (const auto &[id, node] : m_network->network_nodes)
        {
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
                clicked_node_id = id;

                break; // Stop checking, node located
            }
        }

        if (clicked_node_id != -1)
        {
            m_network->network_nodes.erase(clicked_node_id); // Delete node

            // Clear out all associated edges (Thank you Gemini for the concise code. Beats my implementation)
            m_network->network_edges.erase(
                std::remove_if(m_network->network_edges.begin(), m_network->network_edges.end(),
                               [clicked_node_id](const ThermalEdge &e)
                               {
                                   return e.id_0 == clicked_node_id || e.id_1 == clicked_node_id;
                               }),
                m_network->network_edges.end());

            // Clear UI if the deleted node was selected
            if (m_sel_node_id == clicked_node_id)
            {
                m_sel_node_id = -1;
                ((MainFrame *)GetParent())->ShowNodeProperties(-1);
            }
            break; // No need to check edges
        }

        // Now check for edges
        for (size_t i = 0; i < m_network->network_edges.size(); i++)
        {
            const ThermalEdge &edge = m_network->network_edges[i];

            // Convert everything to screen px
            double edge_x_1 = m_network->network_nodes[edge.id_0].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double edge_y_1 = m_network->network_nodes[edge.id_0].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double edge_x_2 = m_network->network_nodes[edge.id_1].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double edge_y_2 = m_network->network_nodes[edge.id_1].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

            // Check first if bounding box criteria is met
            if (in_bounding_box(mouse_pos.x, mouse_pos.y, edge_x_1, edge_y_1, edge_x_2, edge_y_2) && distance_perpendicular(mouse_pos.x, mouse_pos.y, edge_x_1, edge_y_1, edge_x_2, edge_y_2) <= EDGE_SELECTION_TOLERANCE)
            {
                // Vaporize edge and exit loop
                m_network->network_edges.erase(m_network->network_edges.begin() + i);

                break;
            }
        }

        break;
    }
    }

    Refresh(); // Redraw
}

void ThermalCanvas::OnMouseMove(wxMouseEvent &event)
{
    // Check for dragging mouse with valid node & a valid network
    if (m_is_dragging && m_sel_node_id != -1 && m_network)
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

        // Loop through all nodes to find (first) x snaps and (second) y snaps
        // ThermalNode& drag_node = m_network->network_nodes[m_sel_node_id];
        SNAP_X = -1;
        SNAP_Y = -1;
        for (const auto &[id, node] : m_network->network_nodes)
        {
            if (node.node_id == m_sel_node_id)
                continue;

            if (std::abs(node.canvas_position_x - new_norm_x) * (width - 2 * CANVAS_MARGIN) < SNAP_MARGIN)
            {
                // Snap to first match
                SNAP_X = node.canvas_position_x;
                new_norm_x = node.canvas_position_x;
                break;
            }
        }
        for (const auto &[id, node] : m_network->network_nodes)
        {
            if (node.node_id == m_sel_node_id)
                continue;

            if (std::abs(node.canvas_position_y - new_norm_y) * (height - 2 * CANVAS_MARGIN) < SNAP_MARGIN)
            {
                // Snap to first match
                SNAP_Y = node.canvas_position_y;
                new_norm_y = node.canvas_position_y;
                break;
            }
        }

        // Update the actual network data
        m_network->network_nodes[m_sel_node_id].canvas_position_x = new_norm_x;
        m_network->network_nodes[m_sel_node_id].canvas_position_y = new_norm_y;

        // Redraw to make the node follow the cursor
        Refresh();
    }
}

void ThermalCanvas::OnMouseLeftUp(wxMouseEvent &event)
{
    m_is_dragging = false;
    SNAP_X = -1;
    SNAP_Y = -1;
    Refresh();
}

void ThermalCanvas::SetToolMode(ToolMode mode)
{
    m_current_tool = mode;
    m_first_edge_node = -1; // Always reset edge memory when switching tools
    Refresh();
}