#include "ThermalCanvas.h"

wxBEGIN_EVENT_TABLE(ThermalCanvas, wxPanel)
    EVT_PAINT(ThermalCanvas::OnPaint)
        EVT_LEFT_DOWN(ThermalCanvas::OnMouseLeftDown)
            EVT_LEFT_UP(ThermalCanvas::OnMouseLeftUp)
                EVT_MOTION(ThermalCanvas::OnMouseMove)
                    wxEND_EVENT_TABLE()

                        ThermalCanvas::ThermalCanvas(wxWindow *parent)
    : wxPanel(parent, wxID_ANY)
{
    SetBackgroundColour(wxColour(42, 42, 44));
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

    // Adjust node size based on preferences
    NODE_RADIUS = (float)(wxConfigBase::Get()->ReadLong("/UI/NodeRadius", 15));

    // Draw snapping guides
    gc->SetPen(wxPen(COLOR_GUIDE, 1));

    if (SNAP_DX != 0) 
    {
        // Diagonal drawing
        double guideline_x = SNAP_X * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double guideline_y = SNAP_Y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

        if (SNAP_DX > 0) 
        {
            // Down and Right
            auto [x_1, y_1, x_2, y_2] = extended_line(guideline_x, guideline_y, guideline_x + 10, guideline_y + 10, width, height);
            gc->StrokeLine(x_1, y_1, x_2, y_2);
        } 
        else 
        {
            // Up and Right
            auto [x_1, y_1, x_2, y_2] = extended_line(guideline_x, guideline_y, guideline_x + 10, guideline_y - 10, width, height);
            gc->StrokeLine(x_1, y_1, x_2, y_2);
        }
    } 
    else 
    {
        // Orthogonal drawing
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
    }

    // Draw edges, then nodes
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

        gc->SetPen(wxPen(COLOR_EDGE_BACKDROP, 5));
        gc->StrokeLine(x1, y1, x2, y2);

        if (i == m_sel_edge_index)
        {
            gc->SetPen(wxPen(COLOR_SELECT, 4)); // Thick yellow pen
        }
        else
            gc->SetPen(wxPen(COLOR_EDGE, 3));

        gc->StrokeLine(x1, y1, x2, y2);
        double edge_flux = m_network->get_edge_flux(i);
        if (std::abs(edge_flux) > 1e-6) // Flux exists, draw arrow
        {
            double arrow_amplitude = std::min(distance_cartesian(x1, y1, x2, y2) * 0.3 - 10, 6.0);
            auto [cx, cy, dx, dy] = line_info(x1, y1, x2, y2); 
            if (!std::signbit(edge_flux))
            {
                dx *= -1; dy *= -1;
            }
            gc->StrokeLine(cx-arrow_amplitude*dy, cy+arrow_amplitude*dx, cx+arrow_amplitude*dx, cy+arrow_amplitude*dy);
            gc->StrokeLine(cx+arrow_amplitude*dy, cy-arrow_amplitude*dx, cx+arrow_amplitude*dx, cy+arrow_amplitude*dy);
        }
        
    }

    if (m_current_tool == ToolMode::ADD_EDGE && m_first_edge_node != -1)
    {
        // Use a dotted yellow line for the preview
        gc->SetPen(wxPen(COLOR_SELECT, 2, wxPENSTYLE_DOT));

        const ThermalNode &start_node = m_network->network_nodes[m_first_edge_node];
        double start_x = start_node.canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
        double start_y = start_node.canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

        gc->StrokeLine(start_x, start_y, m_current_mouse_pos.x, m_current_mouse_pos.y);
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

        // Find the center of the node
        double center_x = x + NODE_RADIUS;
        double center_y = y + NODE_RADIUS;

        // Heat flux out/in
        if (std::abs(node.ext_load) > 0.0)
        {
            gc->SetPen(wxPen(wxColour(255, 120, 0), 3)); // 3px Orange Pen

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
        if (std::abs(max_node_temperature - min_node_temperature) < 0.01)
        {
            gc->SetBrush(COLOR_DESELECT);
            gc->SetPen(COLOR_DESELECT);
        }
        else
        {
            double rat = (node.node_temperature - min_node_temperature) / (max_node_temperature - min_node_temperature);
            gc->SetBrush(wxBrush(wxColour((int)(200 * rat) + 30, 35, int(200 * (1 - rat)) + 30)));
            gc->SetPen(wxPen(wxColour((int)(200 * rat) + 10, 15, int(200 * (1 - rat)) + 10)));
        }

        // Selection pen
        if ((m_current_tool == ToolMode::SELECT && m_sel_node_ids.count(node.node_id) == 1) || (m_current_tool == ToolMode::ADD_EDGE && m_first_edge_node == node.node_id))
        {
            gc->SetPen(wxPen(COLOR_SELECT, 3)); // Yellow pen
        }
        else if (m_del_node_index == node.node_id)
        {
            gc->SetPen(wxPen(COLOR_SELECT, 3, wxPENSTYLE_DOT)); // Dashed yellow pen
        }

        gc->DrawEllipse(x, y, NODE_RADIUS * 2, NODE_RADIUS * 2);

        // Format the temperature to 1 decimal place
        wxString temp_text = wxString::Format("%s (%.1f C)", node.property_label, node.node_temperature);

        // Text box width, height
        double text_w, text_h;

        gc->GetTextExtent(temp_text, &text_w, &text_h);

        std::vector<ThermalNode> connected_nodes = m_network->connected_nodes(node.node_id);
        std::vector<double> node_rel_angle;

        for (ThermalNode test_node : connected_nodes)
        {
            auto [test_node_x, test_node_y] = test_node.screenCoordinates(width, height, CANVAS_MARGIN);
            node_rel_angle.push_back(std::atan2(test_node_y - center_y, test_node_x - center_x));
        }

        if (std::abs(node.ext_load) > 0.0)
            node_rel_angle.push_back(-PI/2.0);

        std::sort(node_rel_angle.begin(), node_rel_angle.end()); // Sort in ascending order

        double best_angle = 0;
        double max_gap = -1;

        for (size_t i = 0; i < node_rel_angle.size(); i++)
        {
            double a1 = node_rel_angle[i];
            double a2 = node_rel_angle[(i + 1) % node_rel_angle.size()];

            double gap = a2 - a1;
            if (i == node_rel_angle.size() - 1)
                gap += 2 * M_PI;

            if (gap > max_gap)
            {
                max_gap = gap;
                best_angle = a1 + gap / 2.0;
            }
        }

        double angle = best_angle + PI / 2; // 0 = straight up, + = clockwise
        double text_x = center_x - text_w / 2.0 + std::sin(angle) * (NODE_RADIUS + text_w / 2 + 10);
        double text_y = center_y - text_h / 2.0 - std::cos(angle) * (NODE_RADIUS + text_h / 2 + 10);

        // Draw text
        gc->DrawText(temp_text, text_x, text_y);
    }

    // Draw Bounding Box Overlay
    if (m_is_box_selecting)
    {
        // Selection box draw
        gc->SetPen(wxPen(wxColour(50, 150, 255), 1));
        gc->SetBrush(wxBrush(wxColour(50, 150, 255, 50))); // A smidge transparent

        double x = std::min(m_box_start.x, m_current_mouse_pos.x);
        double y = std::min(m_box_start.y, m_current_mouse_pos.y);
        double w = std::abs(m_box_start.x - m_current_mouse_pos.x);
        double h = std::abs(m_box_start.y - m_current_mouse_pos.y);

        gc->DrawRectangle(x, y, w, h);
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

    // Switch for tools
    switch (m_current_tool)
    {
    case ToolMode::SELECT:
    {
        m_sel_edge_index = -1;
        bool found_node = false;

        for (auto const &[id, node] : m_network->network_nodes)
        {
            double node_x = node.canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double node_y = node.canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
            double dx = mouse_pos.x - node_x;
            double dy = mouse_pos.y - node_y;

            if (std::sqrt((dx * dx) + (dy * dy)) <= NODE_RADIUS)
            {
                found_node = true;

                if (event.ShiftDown())
                {
                    // Toggle selection! If it's in the set, remove it. Otherwise, add it.
                    if (m_sel_node_ids.count(id))
                        m_sel_node_ids.erase(id);
                    else
                        m_sel_node_ids.insert(id);
                }
                else
                {
                    // Only clear the group if they clicked a brand new, unselected node
                    if (m_sel_node_ids.count(id) == 0)
                    {
                        m_sel_node_ids.clear();
                        m_sel_node_ids.insert(id);
                    }
                }

                // Dragging
                m_is_dragging = true;
                m_drag_start_mouse = mouse_pos; // Anchor the physical mouse

                m_drag_start_nodes.clear();
                for (int sel_id : m_sel_node_ids)
                {
                    // Anchor the mathematical node positions
                    m_drag_start_nodes[sel_id] = {
                        m_network->network_nodes[sel_id].canvas_position_x,
                        m_network->network_nodes[sel_id].canvas_position_y};
                }

                // Update UI based on how many are selected
                if (m_sel_node_ids.size() == 1)
                {
                    ((MainFrame *)GetParent())->ShowNodeProperties(*m_sel_node_ids.begin());
                }
                else if (m_sel_node_ids.size() > 1)
                {
                    ((MainFrame *)GetParent())->ShowNodeProperties(-2); // Trigger our new multi-select UI!
                }
                else
                {
                    ((MainFrame *)GetParent())->ShowNodeProperties(-1);
                }
                break;
            }
        }

        // Check for edges ONLY if we didn't hit a node
        if (!found_node)
        {
            for (size_t i = 0; i < m_network->network_edges.size(); i++)
            {
                const ThermalEdge &edge = m_network->network_edges[i];
                double edge_x_1 = m_network->network_nodes[edge.id_0].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
                double edge_y_1 = m_network->network_nodes[edge.id_0].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
                double edge_x_2 = m_network->network_nodes[edge.id_1].canvas_position_x * (width - CANVAS_MARGIN * 2) + CANVAS_MARGIN;
                double edge_y_2 = m_network->network_nodes[edge.id_1].canvas_position_y * (height - CANVAS_MARGIN * 2) + CANVAS_MARGIN;

                if (in_bounding_box(mouse_pos.x, mouse_pos.y, edge_x_1, edge_y_1, edge_x_2, edge_y_2) &&
                    distance_perpendicular(mouse_pos.x, mouse_pos.y, edge_x_1, edge_y_1, edge_x_2, edge_y_2) <= EDGE_SELECTION_TOLERANCE)
                {
                    m_sel_node_ids.clear(); // Hitting an edge clears the node group!
                    m_sel_edge_index = i;
                    ((MainFrame *)GetParent())->ShowEdgeProperties(i);
                    break;
                }
            }
        }

        // Clicked absolute empty space -> Start Bounding Box
        if (!found_node && m_sel_edge_index == -1)
        {
            if (!event.ShiftDown())
                m_sel_node_ids.clear(); // Keep existing selection if holding shift

            m_is_box_selecting = true;
            m_box_start = mouse_pos;
            m_current_mouse_pos = mouse_pos;

            ((MainFrame *)GetParent())->ResetPropertiesWindow();
            ((MainFrame *)GetParent())->ShowNodeProperties(-1);
        }
        break;
    }
    case ToolMode::ADD_NODE:
    {
        // Create a node with default parameters and add to network
        double node_temperature = get_default_temperature();
        ThermalNode new_node(norm_x, norm_y, 1.0, 500.0, "New Node", 0, node_temperature);
        int new_node_id = m_network->add_node(new_node);

        // If user is pressing shift, multi add. Otherwise, edit properties
        if (!event.ShiftDown())
        {
            ((MainFrame *)GetParent())->ForceSelectTool(); // Override selection
            m_sel_node_ids.clear();
            m_sel_node_ids.insert(new_node_id);
            ((MainFrame *)GetParent())->ShowNodeProperties(new_node_id);
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
                if (!m_network->has_edge(m_first_edge_node, clicked_node_id)) // Only care if edge doesn't exist
                {
                    m_network->add_edge(ThermalEdge(m_first_edge_node, clicked_node_id, PureResistance{10.0}));
                }
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

        // Node was identified, delete
        if (clicked_node_id != -1)
        {
            m_sel_node_ids.clear();
            m_sel_edge_index = -1;
            m_sel_node_ids.insert(clicked_node_id);

            DeleteSelectedItems();
        }
        else // No node, check for edges
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
                    // Atomize edge and exit loop
                    m_sel_edge_index = i;
                    m_sel_node_ids.clear();

                    DeleteSelectedItems();

                    break;
                }
            }
        }

        break;
    }
    }

    Refresh(); // Redraw
}

void ThermalCanvas::OnMouseMove(wxMouseEvent &event)
{
    if (m_is_dragging && m_sel_node_ids.size() > 0 && m_network)
    {
        int width, height;
        GetClientSize(&width, &height);
        if (width == 0 || height == 0)
            return;

        wxPoint mouse_pos = event.GetPosition();

        // Calculate the mouse movement since the click started
        double total_dx = (double)(mouse_pos.x - m_drag_start_mouse.x) / (width - CANVAS_MARGIN * 2);
        double total_dy = (double)(mouse_pos.y - m_drag_start_mouse.y) / (height - CANVAS_MARGIN * 2);

        // Safely clamp the delta so the group can't be pushed off-screen
        double min_x = 1.0, max_x = 0.0, min_y = 1.0, max_y = 0.0;
        for (int id : m_sel_node_ids)
        {
            auto [start_x, start_y] = m_drag_start_nodes[id];
            min_x = std::min(min_x, start_x);
            max_x = std::max(max_x, start_x);
            min_y = std::min(min_y, start_y);
            max_y = std::max(max_y, start_y);
        }

        if (min_x + total_dx < 0.0)
            total_dx = -min_x;
        if (max_x + total_dx > 1.0)
            total_dx = 1.0 - max_x;
        if (min_y + total_dy < 0.0)
            total_dy = -min_y;
        if (max_y + total_dy > 1.0)
            total_dy = 1.0 - max_y;

        SNAP_X = -1;
        SNAP_Y = -1;
        SNAP_DX = 0;

        // Snapping (Only snap if dragging a single node to avoid catastrophe)
        if (m_sel_node_ids.size() == 1)
        {
            int id = *m_sel_node_ids.begin();
            auto [start_x, start_y] = m_drag_start_nodes[id];

            double theoretical_x = start_x + total_dx;
            double theoretical_y = start_y + total_dy;

            for (const auto &[other_id, node] : m_network->network_nodes)
            {
                if (other_id == id)
                    continue;

                if (SNAP_X == -1 && std::abs(node.canvas_position_x - theoretical_x) * (width - 2 * CANVAS_MARGIN) < SNAP_MARGIN)
                {
                    SNAP_X = node.canvas_position_x;
                    total_dx = SNAP_X - start_x; // Override delta to snap exactly
                }
                if (SNAP_Y == -1 && std::abs(node.canvas_position_y - theoretical_y) * (height - 2 * CANVAS_MARGIN) < SNAP_MARGIN)
                {
                    SNAP_Y = node.canvas_position_y;
                    total_dy = SNAP_Y - start_y;
                }
                double theoretical_x_px = theoretical_x * (width - 2 * CANVAS_MARGIN) + CANVAS_MARGIN;
                double theoretical_y_px = theoretical_y * (height - 2 * CANVAS_MARGIN) + CANVAS_MARGIN;
                double node_x_px = node.canvas_position_x * (width - 2 * CANVAS_MARGIN) + CANVAS_MARGIN;
                double node_y_px = node.canvas_position_y * (height - 2 * CANVAS_MARGIN) + CANVAS_MARGIN;
                if (SNAP_X == -1 && SNAP_Y == -1 && distance_cartesian(theoretical_x_px, theoretical_y_px, node_x_px, node_y_px) >= DISTANCE_NO_SNAP) // Check for diagonal (only if >40px away)
                {
                    // Check Down-Right Diagonal (y + 10)
                    if (distance_perpendicular(theoretical_x_px, theoretical_y_px, node_x_px, node_y_px, node_x_px + 10, node_y_px + 10) <= 2 * SNAP_MARGIN)
                    {
                        auto [snap_px_x, snap_px_y] = enforce_diagonal(theoretical_x_px, theoretical_y_px, node_x_px, node_y_px, node_x_px + 10, node_y_px + 10);
                        
                        // Visual anchors to tgt node
                        SNAP_X = node.canvas_position_x;
                        SNAP_Y = node.canvas_position_y;
                        SNAP_DX = 1; // Down-right
                        
                        // Physical anchors
                        double rel_x = (snap_px_x - CANVAS_MARGIN) / (width - 2 * CANVAS_MARGIN);
                        double rel_y = (snap_px_y - CANVAS_MARGIN) / (height - 2 * CANVAS_MARGIN);
                        total_dx = rel_x - start_x;
                        total_dy = rel_y - start_y;
                    }
                    // Check Up-Right Diagonal (y - 10)
                    else if (distance_perpendicular(theoretical_x_px, theoretical_y_px, node_x_px, node_y_px, node_x_px + 10, node_y_px - 10) <= 2 * SNAP_MARGIN)
                    {
                        auto [snap_px_x, snap_px_y] = enforce_diagonal(theoretical_x_px, theoretical_y_px, node_x_px, node_y_px, node_x_px + 10, node_y_px - 10);
                        
                        // Visual anchors to tgt node
                        SNAP_X = node.canvas_position_x;
                        SNAP_Y = node.canvas_position_y;
                        SNAP_DX = -1; // Up-right
                        
                        // Physical anchors
                        double rel_x = (snap_px_x - CANVAS_MARGIN) / (width - 2 * CANVAS_MARGIN);
                        double rel_y = (snap_px_y - CANVAS_MARGIN) / (height - 2 * CANVAS_MARGIN);
                        total_dx = rel_x - start_x;
                        total_dy = rel_y - start_y;
                    }
                }
            }
        }

        // Apply the final calculated delta to the stored anchor positions
        for (int id : m_sel_node_ids)
        {
            auto [start_x, start_y] = m_drag_start_nodes[id];
            m_network->network_nodes[id].canvas_position_x = start_x + total_dx;
            m_network->network_nodes[id].canvas_position_y = start_y + total_dy;
        }

        Refresh();
    }
    else if (m_is_box_selecting || (m_current_tool == ToolMode::ADD_EDGE && m_first_edge_node != -1))
    { // Bounding box
        m_current_mouse_pos = event.GetPosition();
        Refresh(); // Force OnPaint to draw the overlay
    }
    else if (!m_is_dragging && m_current_tool == ToolMode::DELETE_ITEM)
    { // Check for nodes

        int width, height;
        GetClientSize(&width, &height);
        wxPoint mouse_pos = event.GetPosition();
        double mouse_pos_x = (double)(mouse_pos.x);
        double mouse_pos_y = (double)(mouse_pos.y);
        double dist;

        for (auto& [id, node] : m_network->network_nodes)
        {
            auto [node_x_px, node_y_px] = node.screenCoordinates(width, height, CANVAS_MARGIN);
            dist = std::sqrt((node_x_px - mouse_pos_x) * (node_x_px - mouse_pos_x) + (node_y_px - mouse_pos_y) * (node_y_px - mouse_pos_y));
            if (dist <= NODE_RADIUS)
            {
                m_del_node_index = id;
                break;
            }
        }

        // No nodes hovered
        m_del_node_index = -1;
    }
}

void ThermalCanvas::OnMouseLeftUp(wxMouseEvent &event)
{
    m_is_dragging = false;
    SNAP_X = -1;
    SNAP_Y = -1;
    SNAP_DX = 0;

    if (m_is_box_selecting)
    {
        m_is_box_selecting = false;

        int width, height;
        GetClientSize(&width, &height);
        if (width == 0 || height == 0)
            return;

        // Calculate the pixel boundaries of the drawn box
        double min_x = std::min(m_box_start.x, event.GetPosition().x);
        double max_x = std::max(m_box_start.x, event.GetPosition().x);
        double min_y = std::min(m_box_start.y, event.GetPosition().y);
        double max_y = std::max(m_box_start.y, event.GetPosition().y);

        // Convert to internal normalized fractions
        double norm_min_x = (min_x - CANVAS_MARGIN) / (width - CANVAS_MARGIN * 2);
        double norm_max_x = (max_x - CANVAS_MARGIN) / (width - CANVAS_MARGIN * 2);
        double norm_min_y = (min_y - CANVAS_MARGIN) / (height - CANVAS_MARGIN * 2);
        double norm_max_y = (max_y - CANVAS_MARGIN) / (height - CANVAS_MARGIN * 2);

        // Sweep the network for nodes caught inside
        for (const auto &[id, node] : m_network->network_nodes)
        {
            if (node.canvas_position_x >= norm_min_x && node.canvas_position_x <= norm_max_x &&
                node.canvas_position_y >= norm_min_y && node.canvas_position_y <= norm_max_y)
            {
                m_sel_node_ids.insert(id);
            }
        }

        // Update the properties panel
        if (m_sel_node_ids.size() == 1)
        {
            ((MainFrame *)GetParent())->ShowNodeProperties(*m_sel_node_ids.begin());
        }
        else if (m_sel_node_ids.size() > 1)
        {
            ((MainFrame *)GetParent())->ShowNodeProperties(-2); // Multiple selected!
        }
    }

    Refresh();
}

void ThermalCanvas::SetToolMode(ToolMode mode)
{
    m_first_edge_node = -1; // Always reset edge+sel memory when switching tools
    m_sel_edge_index = -1;
    m_sel_node_ids.clear();
    m_current_tool = mode;
    Refresh();
}

void ThermalCanvas::DeleteSelectedItems()
{
    if (m_sel_node_ids.empty() && m_sel_edge_index == -1) return;

    // Delete the edges first
    m_network->network_edges.erase(
        std::remove_if(
            m_network->network_edges.begin(), m_network->network_edges.end(),
            [&](ThermalEdge& e){
                bool connectedToDeletedNode = m_sel_node_ids.count(e.id_0) || m_sel_node_ids.count(e.id_1);
                bool markedForDeletion = (m_sel_edge_index != -1 && &e == &m_network->network_edges[m_sel_edge_index]);
                return connectedToDeletedNode || markedForDeletion;
            }
        ),
        m_network->network_edges.end());

    // Delete the nodes
    for (int id : m_sel_node_ids)
    {
        m_network->network_nodes.erase(id);
    }

    // Clear memory and UI
    m_sel_node_ids.clear();
    m_sel_edge_index = -1;
    ((MainFrame*)GetParent())->ResetPropertiesWindow();
    Refresh();
}

void ThermalCanvas::CopySelected()
{
    m_clipboard_nodes.clear();
    for (int id : m_sel_node_ids) {
        m_clipboard_nodes.push_back(m_network->network_nodes[id]);
    }
}

void ThermalCanvas::PasteSelected()
{
    if (m_clipboard_nodes.empty() || !m_network) return;

    m_sel_node_ids.clear(); // Deselect originals
    m_sel_edge_index = -1;

    for (ThermalNode& node : m_clipboard_nodes) {
        // Shift the node so it doesn't perfectly hide under the original
        ThermalNode new_node = node;
        new_node.canvas_position_x = std::clamp(new_node.canvas_position_x + 0.03, 0.0, 1.0);
        new_node.canvas_position_y = std::clamp(new_node.canvas_position_y + 0.03, 0.0, 1.0);
        
        // Add to network and select the newly generated ID
        int new_id = m_network->add_node(new_node);
        m_sel_node_ids.insert(new_id);
        
        // Shift the clipboard node so pasting AGAIN continues the diagonal pattern!
        node.canvas_position_x = new_node.canvas_position_x;
        node.canvas_position_y = new_node.canvas_position_y;
    }

    // Update UI
    if (m_sel_node_ids.size() == 1) ((MainFrame*)GetParent())->ShowNodeProperties(*m_sel_node_ids.begin());
    else if (m_sel_node_ids.size() > 1) ((MainFrame*)GetParent())->ShowNodeProperties(-2);
    
    Refresh();
}