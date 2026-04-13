#pragma once
#include <wx/wx.h>
#include <wx/graphics.h>
#include <vector>
#include <string>
#include <memory>

struct TransientDataSeries {
    std::string label;
    std::vector<double> values;
    wxColour color;
};

class TransientPreview : public wxPanel {
public:
    TransientPreview(wxWindow* parent);
    bool LoadCSV(const std::string& path);

private:
    std::vector<double> m_time_steps;
    std::vector<TransientDataSeries> m_node_series;
    std::vector<TransientDataSeries> m_edge_series;

    // View state for future zooming/panning
    double m_x_min = 0, m_x_max = 1;
    double m_y_min_nodes = 0, m_y_max_nodes = 100;
    double m_y_min_edges = -10, m_y_max_edges = 10;

    void OnPaint(wxPaintEvent& evt);
    void DrawChart(wxGraphicsContext* gc, const wxRect& rect, 
                   const std::vector<TransientDataSeries>& series,
                   double y_min, double y_max, const std::string& title);

    wxDECLARE_EVENT_TABLE();
};