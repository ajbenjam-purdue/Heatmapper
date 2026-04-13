#include "TransientPreview.h"
#include "utils.h"
#include <wx/dcbuffer.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

wxBEGIN_EVENT_TABLE(TransientPreview, wxPanel)
    EVT_PAINT(TransientPreview::OnPaint)
wxEND_EVENT_TABLE()

TransientPreview::TransientPreview(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(*wxWHITE);
}

bool TransientPreview::LoadCSV(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;

    m_time_steps.clear();
    m_node_series.clear();
    m_edge_series.clear();

    std::string line;
    if (!std::getline(file, line)) return false;

    // Parse header
    std::stringstream ss(line);
    std::string header;
    std::vector<std::string> headers;
    while (std::getline(ss, header, ',')) {
        headers.push_back(header);
    }

    if (headers.empty()) return false;

    // Initialize series based on headers
    // Time (s) is always column 0 per the prog's output
    // TODO: verification code to ensure conformity
    for (size_t i = 1; i < headers.size(); ++i) {
        TransientDataSeries series;
        series.label = headers[i];
        series.color = map_color_round_robin((int)i - 1);
        
        if (series.label.find("[C]") != std::string::npos) {
            m_node_series.push_back(series);
        } else if (series.label.find("[W]") != std::string::npos) {
            m_edge_series.push_back(series);
        }
    }

    // Parse data
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss_line(line);
        std::string val_str;
        
        // Time
        if (std::getline(ss_line, val_str, ',')) {
            m_time_steps.push_back(std::stod(val_str));
        }

        size_t node_idx = 0;
        size_t edge_idx = 0;

        for (size_t i = 1; i < headers.size(); ++i) {
            if (std::getline(ss_line, val_str, ',')) {
                double val = std::stod(val_str);
                if (headers[i].find("[C]") != std::string::npos) {
                    m_node_series[node_idx++].values.push_back(val);
                } else if (headers[i].find("[W]") != std::string::npos) {
                    m_edge_series[edge_idx++].values.push_back(val);
                }
            }
        }
    }

    // Update bounds
    if (!m_time_steps.empty()) {
        m_x_min = m_time_steps.front();
        m_x_max = m_time_steps.back();
    }

    auto update_y_bounds = [](const std::vector<TransientDataSeries>& series, double& y_min, double& y_max) {
        bool first = true;
        for (const auto& s : series) {
            for (double v : s.values) {
                if (first) {
                    y_min = y_max = v;
                    first = false;
                } else {
                    y_min = std::min(y_min, v);
                    y_max = std::max(y_max, v);
                }
            }
        }
        // Add some padding
        double range = y_max - y_min;
        if (range == 0) range = 1.0;
        y_min -= 0.1 * range;
        y_max += 0.1 * range;
    };

    update_y_bounds(m_node_series, m_y_min_nodes, m_y_max_nodes);
    update_y_bounds(m_edge_series, m_y_min_edges, m_y_max_edges);

    Refresh();
    return true;
}

void TransientPreview::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
    if (!gc) return;

    wxSize size = GetClientSize();
    int margin = 40;
    int chart_height = (size.y - 3 * margin) / 2;

    wxRect node_rect(margin, margin, size.x - 2 * margin, chart_height);
    wxRect edge_rect(margin, 2 * margin + chart_height, size.x - 2 * margin, chart_height);

    DrawChart(gc.get(), node_rect, m_node_series, m_y_min_nodes, m_y_max_nodes, "Node Temperatures [C]");
    DrawChart(gc.get(), edge_rect, m_edge_series, m_y_min_edges, m_y_max_edges, "Edge Heat Fluxes [W]");
}

void TransientPreview::DrawChart(wxGraphicsContext* gc, const wxRect& rect, 
                                 const std::vector<TransientDataSeries>& series,
                                 double y_min, double y_max, const std::string& title)
{
    // Draw background/border
    gc->SetBrush(*wxWHITE_BRUSH);
    gc->SetPen(*wxBLACK_PEN);
    gc->DrawRectangle(rect.x, rect.y, rect.width, rect.height);

    // Title
    gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD), *wxBLACK);
    double tw, th;
    gc->GetTextExtent(title, &tw, &th);
    gc->DrawText(title, rect.x + (rect.width - tw) / 2, rect.y - th - 5);

    if (m_time_steps.empty() || series.empty()) return;

    auto to_screen_x = [&](double x) {
        return rect.x + (x - m_x_min) / (m_x_max - m_x_min) * rect.width;
    };
    auto to_screen_y = [&](double y) {
        return rect.y + rect.height - (y - y_min) / (y_max - y_min) * rect.height;
    };

    // Draw grid/axes (simplified)
    gc->SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));
    for (int i = 0; i <= 4; ++i) {
        double y_val = y_min + (y_max - y_min) * i / 4.0;
        double sy = to_screen_y(y_val);
        gc->StrokeLine(rect.x, sy, rect.x + rect.width, sy);
        
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << y_val;
        gc->DrawText(ss.str(), rect.x - 35, sy - 7);
    }

    // Draw series
    for (const auto& s : series) {
        if (s.values.empty()) continue;
        
        gc->SetPen(wxPen(s.color, 2));
        wxGraphicsPath path = gc->CreatePath();
        path.MoveToPoint(to_screen_x(m_time_steps[0]), to_screen_y(s.values[0]));
        
        for (size_t i = 1; i < m_time_steps.size(); ++i) {
            path.AddLineToPoint(to_screen_x(m_time_steps[i]), to_screen_y(s.values[i]));
        }
        gc->StrokePath(path);
    }
}