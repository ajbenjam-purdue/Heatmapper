#pragma once
#include "util.h"
#include <cmath>
#include <algorithm>

// Perpendicular distance from the point (x0, y0) to the line between (x1, y1) and (x2, y2)
double distance_perpendicular(double x0, double y0, double x1, double y1, double x2, double y2)
{
    return std::abs((y2 - y1) * x0 - (x2 - x1) * y0 + x2 * y1 - y2 * x1) / std::sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1));
}

// Returns true if the point (x0, y0) lies in the bounding box from (x1, y1) to (x2, y2)
bool in_bounding_box(double x0, double y0, double x1, double y1, double x2, double y2)
{
    double tol = 10.0; // pixels
    double x_l = std::min(x1, x2) - tol;
    double x_h = std::max(x1, x2) + tol;
    double y_l = std::min(y1, y2) - tol;
    double y_h = std::max(y1, y2) + tol;

    return (x0 > x_l && x0 < x_h && y0 > y_l && y0 < y_h);
}

// Cartesian distance between two points
double distance_cartesian(double x0, double y0, double x1, double y1)
{
    return std::sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
}