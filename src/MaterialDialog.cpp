#include "MaterialDialog.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

wxBEGIN_EVENT_TABLE(MaterialDialog, wxDialog)
    EVT_LISTBOX(ID_MaterialList, MaterialDialog::OnListSelected)
    EVT_BUTTON(ID_AddMaterial, MaterialDialog::OnAddMaterial)
    EVT_BUTTON(ID_DeleteMaterial, MaterialDialog::OnDeleteMaterial)
    // We bind all text edits to the same function so we update the struct instantly!
    EVT_TEXT(ID_PropertyEdited, MaterialDialog::OnPropertyEdited) 
    EVT_BUTTON(ID_DuplicateMaterial, MaterialDialog::OnDuplicateMaterial)
wxEND_EVENT_TABLE()

MaterialDialog::MaterialDialog(wxWindow* parent, MaterialLibrary mat_lib) 
    : wxDialog(parent, wxID_ANY, "Discretize Node", wxDefaultPosition, wxSize(550, 400)),
      m_mat_lib(mat_lib)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Inner left/right split
    wxBoxSizer* content_sizer = new wxBoxSizer(wxHORIZONTAL);

    // Left: List and buttons to mod the list
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
    
    // List of mats
    m_material_list = new wxListBox(this, ID_MaterialList, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);
    
    // Build out list
    for (Material material : m_mat_lib.materials)
    {
        m_material_list->Append(material.name);
    }
    
    left_sizer->Add(m_material_list, 1, wxEXPAND | wxALL, 5); // stretch vertically

    // [+] [d] [-] buttons
    wxBoxSizer* list_btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* btn_add = new wxButton(this, ID_AddMaterial, "+ Add", wxDefaultPosition, wxSize(60, -1));
    wxButton* btn_dup = new wxButton(this, ID_DuplicateMaterial, "⧉ Duplicate", wxDefaultPosition, wxSize(80, -1));
    wxButton* btn_del = new wxButton(this, ID_DeleteMaterial, "- Delete", wxDefaultPosition, wxSize(70, -1));
    list_btn_sizer->Add(btn_add, 0, wxRIGHT, 5);
    list_btn_sizer->Add(btn_dup, 0, wxRIGHT, 5);
    list_btn_sizer->Add(btn_del, 0);
    
    left_sizer->Add(list_btn_sizer, 0, wxALIGN_CENTER | wxALL, 5);

    // Add the whole left panel to the content split
    content_sizer->Add(left_sizer, 1, wxEXPAND | wxALL, 5);

    // Right: Properties
    wxStaticBoxSizer* right_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Material Properties");

    // Build input rows
    m_name_input = AddPropertyRow(right_sizer, "Name:");
    m_density_input = AddPropertyRow(right_sizer, "Density [kg/m3]:");
    m_cp_input = AddPropertyRow(right_sizer, "Specific Heat [J/kg-K]:");
    m_k_input = AddPropertyRow(right_sizer, "Thermal Cond. [W/m-K]:");

    // Add the right panel to the content split
    content_sizer->Add(right_sizer, 2, wxEXPAND | wxALL, 5);

    main_sizer->Add(content_sizer, 1, wxEXPAND | wxALL, 5);
    
    // Ok/Cancel buttons
    main_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(main_sizer);

    // Initial state setup
    if (m_material_list->GetCount() > 0) {
        m_material_list->SetSelection(0);
        wxCommandEvent dummy;
        OnListSelected(dummy); // Force the right side to populate with the first item
    }
}

wxTextCtrl* MaterialDialog::AddPropertyRow(wxSizer* parent_sizer, const wxString& label) {
    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(new wxStaticText(this, wxID_ANY, label), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    // assign ID_PropertyEdited to EVERY text box
    wxTextCtrl* input = new wxTextCtrl(this, ID_PropertyEdited, "");
    row->Add(input, 2, wxEXPAND);
    
    parent_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 10);
    return input;
}

void MaterialDialog::OnListSelected(wxCommandEvent& event)
{
    m_current_selection = m_material_list->GetSelection();
    updateInputs();
}

void MaterialDialog::OnAddMaterial(wxCommandEvent& event)
{
    size_t idx = m_mat_lib.push_material(Material("New Material", 1000.0, 1000.0, 100.0));
    m_material_list->Append("New Material");
    m_material_list->SetSelection(idx);
    updateInputs();
}

void MaterialDialog::OnDuplicateMaterial(wxCommandEvent& event)
{
    m_mat_lib.insert_material(m_mat_lib.materials.at(m_current_selection), m_current_selection + 1);
    m_material_list->Insert(m_mat_lib.materials.at(m_current_selection).name, m_current_selection + 1);
    m_material_list->SetSelection(m_current_selection + 1);
    updateInputs();
}

void MaterialDialog::OnDeleteMaterial(wxCommandEvent& event)
{
    if (m_current_selection != -1)
    {
        m_material_list->Delete(m_current_selection);
        m_mat_lib.materials.erase(m_mat_lib.materials.begin()+m_current_selection);
        m_current_selection -= 1; // Go up one
    }
    updateInputs();
}

void MaterialDialog::OnPropertyEdited(wxCommandEvent& event)
{

}

void MaterialDialog::updateInputs()
{
    if (m_current_selection >= 0 && m_current_selection < m_mat_lib.materials.size())
    {
        m_name_input->SetValue(wxString::Format("%s", m_mat_lib.materials.at(m_current_selection).name));
        m_density_input->SetValue(wxString::Format("%.1f", m_mat_lib.materials.at(m_current_selection).density));
        m_cp_input->SetValue(wxString::Format("%.1f", m_mat_lib.materials.at(m_current_selection).specific_heat));
        m_k_input->SetValue(wxString::Format("%.1f", m_mat_lib.materials.at(m_current_selection).thermal_conductivity));
    }
}