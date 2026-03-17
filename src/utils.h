#pragma once
#include <cmath>
#include <algorithm>
#include <tuple>
#include <vector>
#include <wx/wx.h>
#include <wx/config.h>

const double PI = 3.14159265358979323846; 

// Perpendicular distance from the point (x0, y0) to the line between (x1, y1) and (x2, y2)
inline double distance_perpendicular(double x0, double y0, double x1, double y1, double x2, double y2)
{
    return std::abs((y2 - y1) * x0 - (x2 - x1) * y0 + x2 * y1 - y2 * x1) / std::sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1));
}

// Returns true if the point (x0, y0) lies in the bounding box from (x1, y1) to (x2, y2)
inline bool in_bounding_box(double x0, double y0, double x1, double y1, double x2, double y2)
{
    double tol = 10.0; // pixels
    double x_l = std::min(x1, x2) - tol;
    double x_h = std::max(x1, x2) + tol;
    double y_l = std::min(y1, y2) - tol;
    double y_h = std::max(y1, y2) + tol;

    return (x0 > x_l && x0 < x_h && y0 > y_l && y0 < y_h);
}

// Cartesian distance between two points
inline double distance_cartesian(double x0, double y0, double x1, double y1)
{
    return std::sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
}

// Returns the closest point to the point (x0, y0) which lies on the line between (x1, y1) and (x2, y2).
// Don't use with vertical lines
// Reference: https://stackoverflow.com/questions/5204619/
inline std::pair<double, double> enforce_diagonal(double x0, double y0, double x1, double y1, double x2, double y2)
{
    double m1 = (y2 - y1) / (x2 - x1);
    double m2 = -1.0 / m1;
    double x = (m1 * x1 - m2 * x0 - y1 + y0) / (m1 - m2);
    double y = m2 * (x - x0) + y0;
    return std::pair<double, double>(x, y);
}

// Returns (center x, center y, dx [norm to 1], dy [norm to 1]) for the line between (x1, y1) and (x2, y2).
inline std::tuple<double, double, double, double> line_info(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double r = std::sqrt(dx * dx + dy * dy);
    dx /= r;
    dy /= r;
    return std::tuple((x1 + x2) / 2.0, (y1 + y2) / 2.0, dx, dy);
}

// Returns the points (xa, ya, xb, yb) which lies on the boundary provided and the extension of the line between (x0, y0), (x1, y1)
inline std::tuple<double, double, double, double> extended_line(double x0, double y0, double x1, double y1, double boundary_x, double boundary_y)
{
    double dx = x1 - x0;
    double dy = y1 - y0;

    std::vector<std::pair<double,double>> hits;

    auto try_add = [&](double t)
    {
        double x = x0 + t*dx;
        double y = y0 + t*dy;

        if (x >= 0 && x <= boundary_x &&
            y >= 0 && y <= boundary_y)
        {
            hits.emplace_back(x,y);
        }
    };

    if (dx != 0)
    {
        // x = 0, x = boundary_x
        try_add((0 - x0)/dx);
        try_add((boundary_x - x0)/dx);
    }

    if (dy != 0)
    {
        // y = 0, y = boundary_y
        try_add((0 - y0)/dy);
        try_add((boundary_y - y0)/dy);
    }

    // Should have exactly two intersections for a valid line
    if (hits.size() < 2)
        return {0,0,0,0};

    return {hits[0].first, hits[0].second,
            hits[1].first, hits[1].second};
}

// Returns the default temperature of a node, provided it exists in settings.
inline double get_default_temperature()
{
    double default_temp;
    wxConfigBase::Get()->Read("/Sim/DefaultAmbient", &default_temp, 15.0);
    return default_temp;
}

inline wxColour darken_by(wxColour color, int amount)
{
    int red = color.GetRed() - amount;
    int green = color.GetGreen() - amount;
    int blue = color.GetBlue() - amount;
    return wxColor(std::max(red,0), std::max(green,0), std::max(blue,0));
}

// Returns the lerped color for a given ratio
inline wxColour map_color(double ratio) // Ratio expressed 0.0-1.0
{
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    wxString color_map;
    wxConfigBase::Get()->Read("/UI/NodeScheme", &color_map, "Magma");

    // Colormaps courtesy of matplotlib: All are L* linear
    // 16-point Viridis
    static const unsigned char viridis[16][3] = {
        {68, 1, 84}, {72, 35, 116}, {64, 67, 135}, {52, 94, 141},
        {41, 120, 142}, {32, 144, 140}, {34, 167, 132}, {59, 187, 116},
        {103, 205, 89}, {161, 220, 61}, {218, 233, 50}, {240, 249, 33},
        {250, 252, 38}, {253, 255, 43}, {254, 255, 50}, {253, 255, 56}
    };

    // 16-point Plasma
    static const unsigned char plasma[16][3] = {
        {12, 7, 134}, {49, 18, 171}, {91, 36, 179}, {136, 53, 169},
        {180, 72, 147}, {215, 91, 123}, {239, 118, 93}, {249, 153, 64},
        {247, 189, 42}, {237, 221, 38}, {224, 242, 43}, {209, 255, 57},
        {190, 255, 80}, {170, 255, 104}, {149, 255, 130}, {127, 255, 156}
    };

    // 16-point Magma
    static const unsigned char magma[16][3] = {
        {0, 0, 4}, {28, 16, 68}, {59, 25, 99}, {87, 37, 130},
        {120, 52, 161}, {157, 73, 184}, {193, 100, 199}, {224, 132, 209},
        {245, 170, 208}, {252, 210, 182}, {253, 238, 156}, {251, 250, 123},
        {247, 253, 91}, {242, 254, 61}, {237, 254, 36}, {233, 252, 19}
    };

    const unsigned char (*map)[3] = nullptr;
    color_map = color_map.Trim(true).Trim(false);

    if (color_map.IsSameAs("Viridis", false))
        map = viridis;
    else if (color_map.IsSameAs("Plasma", false))
        map = plasma;
    else if (color_map.IsSameAs("Magma", false))
        map = magma;
    else
    {
        return wxColour(255, 0, 0);
    }

    // Interpolate
    const int n = 15; // number of intervals
    double f_index = ratio * n;
    int idx = static_cast<int>(f_index);
    if (idx >= n) idx = n - 1;
    double t = f_index - idx;

    unsigned char r = static_cast<unsigned char>((1.0 - t) * map[idx][0] + t * map[idx + 1][0]);
    unsigned char g = static_cast<unsigned char>((1.0 - t) * map[idx][1] + t * map[idx + 1][1]);
    unsigned char b = static_cast<unsigned char>((1.0 - t) * map[idx][2] + t * map[idx + 1][2]);

    return wxColour(r, g, b);
}