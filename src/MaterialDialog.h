#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>
#include "MaterialLibrary.h"

enum {
    ID_MaterialList = 2000,
    ID_AddMaterial,
    ID_DeleteMaterial,
    ID_PropertyEdited,
    ID_DuplicateMaterial,
    ID_LoadMaterialsFromJson
};

class MaterialDialog : public wxDialog {
public:
    MaterialDialog(wxWindow* parent, MaterialLibrary mat_lib);

    MaterialLibrary GetModifiedLibrary() const { return m_mat_lib; }

private:
    MaterialLibrary m_mat_lib;   // local copy to edit
    wxListBox* m_material_list;  // actual list
    wxTextCtrl* m_name_input;    // Plaintext name
    wxTextCtrl* m_density_input; // Density
    wxTextCtrl* m_cp_input; // Specific heat
    wxTextCtrl* m_k_input;  // Thermal cond

    int m_current_selection = -1;

    // Helper to build the right-side text boxes
    wxTextCtrl* AddPropertyRow(wxSizer* parent_sizer, const wxString& label);

    // Update inputs with material at index id
    void updateInputs();

    // Event Handlers
    // Clicked on the list
    void OnListSelected(wxCommandEvent& event);
    // Create new empty material
    void OnAddMaterial(wxCommandEvent& event);
    // Create a similar material
    void OnDuplicateMaterial(wxCommandEvent& event);
    // Delete current material
    void OnDeleteMaterial(wxCommandEvent& event);
    // Edited a property
    void OnPropertyEdited(wxCommandEvent& event);
    // Imported a file
    void loadFromJson(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};