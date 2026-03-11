#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include "MaterialLibrary.h"

class MaterialDialog : public wxDialog {
public:
    MaterialDialog(wxWindow* parent, MaterialLibrary mat_lib);

private:
    MaterialLibrary m_mat_lib; 

    // Update inputs with material at index id
    void update_inputs(size_t id);

    // Create new material
    void create_material();

    wxDECLARE_EVENT_TABLE();
};