#pragma once
#include <string>
#include <wx/config.h>
#include <wx/string.h>

class Preferences
{
public:
    static double get_parameter_double(const std::string& param, double default_value);
    static int get_parameter_int(const std::string& param, int default_value);
    static wxString get_parameter_string(const std::string& param, const std::string& default_value);
};