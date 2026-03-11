#include "MaterialDialog.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

enum {
    // ID_TypeChoice = 1001
};

wxBEGIN_EVENT_TABLE(MaterialDialog, wxDialog)
wxEND_EVENT_TABLE()

MaterialDialog::MaterialDialog(wxWindow* parent, MaterialLibrary mat_lib) 
    : wxDialog(parent, wxID_ANY, "Discretize Node", wxDefaultPosition, wxSize(450, 400)),
      m_mat_lib(mat_lib)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    
    SetSizer(main_sizer);
}