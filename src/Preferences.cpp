#include "Preferences.h"

double Preferences::get_parameter_double(const std::string& param, double default_value)
{
    return wxConfigBase::Get()->ReadDouble(wxString(param), default_value);
}

int Preferences::get_parameter_int(const std::string& param, int default_value)
{
    return (int)(wxConfigBase::Get()->ReadLong(wxString(param), default_value));
}

wxString Preferences::get_parameter_string(const std::string& param, const std::string& default_value)
{
    return wxConfigBase::Get()->Read(wxString(param), wxString(default_value));
}