#include "EdgeConfigDialog.h"

// Assign specific IDs for the two types of selectors
enum {
    ID_TypeChoice = 1001,
    ID_MatChoice = 1002
};

wxBEGIN_EVENT_TABLE(EdgeConfigDialog, wxDialog)
    EVT_CHOICE(ID_MatChoice, EdgeConfigDialog::OnMaterialChange)
    EVT_CHOICE(ID_TypeChoice, EdgeConfigDialog::OnTypeChange)
    EVT_BUTTON(wxID_OK, EdgeConfigDialog::OnOK)
wxEND_EVENT_TABLE()

EdgeConfigDialog::EdgeConfigDialog(wxWindow* parent, const MaterialLibrary& mat_lib) 
    : wxDialog(parent, wxID_ANY, "Configure Edge Resistance", wxDefaultPosition, wxSize(450, 500)),
      m_mat_lib(mat_lib)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Dropdown menu
    wxArrayString choices;
    for (const ConfigType& cfType : ConfigTypes)
    {
        choices.Add(cfType.label);
    }
    m_type_choice = new wxChoice(this, ID_TypeChoice, wxDefaultPosition, wxDefaultSize, choices);
    m_type_choice->SetSelection(0);
    main_sizer->Add(m_type_choice, 0, wxEXPAND | wxALL, 10);

    // SVG Image Placeholder
    m_svg_display = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    main_sizer->Add(m_svg_display, 0, wxALIGN_CENTER | wxALL, 10);

    // Warning Text
    m_warning_text = new wxStaticText(this, wxID_ANY, "");
    m_warning_text->SetForegroundColour(*wxRED);
    main_sizer->Add(m_warning_text, 0, wxALIGN_CENTER | wxBOTTOM, 5);

    // Dynamic Sizer for text boxes
    m_dynamic_input_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(m_dynamic_input_sizer, 1, wxEXPAND | wxALL, 10);

    // Buttons
    main_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(main_sizer);

    // Build the initial state
    BuildInputs(0); 
}

wxTextCtrl* EdgeConfigDialog::AddInputRow(const wxString& label, const std::string& key)
{
    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

    row->Add(new wxStaticText(this, wxID_ANY, label), 1, wxALIGN_CENTER_VERTICAL);

    wxTextCtrl* input = new wxTextCtrl(this, wxID_ANY, "1.0");

    input->Bind(wxEVT_TEXT, &EdgeConfigDialog::OnInputChanged, this);

    row->Add(input, 1, wxEXPAND);

    m_dynamic_input_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 5);

    m_inputs[key] = input;
    return input;
}

void EdgeConfigDialog::ValidateInputs()
{
    int sel = m_type_choice->GetSelection();
    m_warning_text->SetLabel("");

    ConfigType selectedConfigType = ConfigTypes.at(sel);
    std::string config_warning = selectedConfigType.is_valid(m_inputs);

    if (config_warning != "")
        m_warning_text->SetLabel(config_warning);

    Layout();
}

void EdgeConfigDialog::OnInputChanged(wxCommandEvent& event)
{
    ValidateInputs();
    event.Skip();
}

void EdgeConfigDialog::BuildInputs(int selection) {
    m_dynamic_input_sizer->Clear(true);
    m_inputs.clear();
    m_k_input = nullptr;
    m_material_choice = nullptr;

    ConfigType selectedConfigType = ConfigTypes.at(selection);
    bool show_k = selectedConfigType.has_property_k();

    if (show_k)
    {
        wxBoxSizer* mat_row = new wxBoxSizer(wxHORIZONTAL);
        mat_row->Add(new wxStaticText(this, wxID_ANY, "Material:"), 1, wxALIGN_CENTER_VERTICAL);
        
        wxArrayString mat_choices;
        for (const auto& mat : m_mat_lib.materials) {
            mat_choices.Add(mat.name);
        }
        mat_choices.Add("Custom...");
        
        m_material_choice = new wxChoice(this, ID_MatChoice, wxDefaultPosition, wxDefaultSize, mat_choices);
        m_material_choice->SetSelection(0);
        mat_row->Add(m_material_choice, 1, wxEXPAND);
        m_dynamic_input_sizer->Add(mat_row, 0, wxEXPAND | wxBOTTOM, 5);
    }

    // Image display
    if (selectedConfigType.image_name != "")
    {
        m_svg_display->SetBitmap(
            wxBitmapBundle::FromSVGFile(selectedConfigType.get_svg_path(), wxSize(200, 150)).GetBitmapFor(this)
        );
    }

    std::string k_label = "Thermal Cond. (k) [W/mK]";

    // Input generation
    for (auto [property_label, property_key] : selectedConfigType.needed_properties)
    {
        if (property_key != "k")
            AddInputRow(property_label, property_key);
        else
            k_label = property_label;
    }
    
    if (show_k)
    {
        wxBoxSizer* k_row = new wxBoxSizer(wxHORIZONTAL);
        k_row->Add(new wxStaticText(this, wxID_ANY, k_label), 1, wxALIGN_CENTER_VERTICAL);
        m_k_input = new wxTextCtrl(this, wxID_ANY, "");
        m_inputs["k"] = m_k_input; // Push to array so math logic still finds it at the end
        k_row->Add(m_k_input, 1, wxEXPAND);
        m_dynamic_input_sizer->Add(k_row, 0, wxEXPAND | wxBOTTOM, 5);
    }

    // Trigger the material change to auto-fill the k box on load
    wxCommandEvent dummy;
    OnMaterialChange(dummy);

    Layout();
}

void EdgeConfigDialog::OnTypeChange(wxCommandEvent& event) {
    BuildInputs(m_type_choice->GetSelection());
}

void EdgeConfigDialog::OnMaterialChange(wxCommandEvent& event) {
    if (!m_material_choice || !m_k_input) return;

    int sel = m_material_choice->GetSelection();
    
    // If a valid library material is picked
    if (sel >= 0 && sel < (int)m_mat_lib.materials.size()) {
        double k = m_mat_lib.materials[sel].thermal_conductivity;
        m_k_input->SetValue(wxString::Format("%.2f", k));
        m_k_input->Disable(); // Disable the input
    } 
    else {
        // "Custom..." selected
        m_k_input->Enable(); // Unlock it
        m_k_input->Clear();
        m_k_input->SetFocus();
    }
}

void EdgeConfigDialog::OnOK(wxCommandEvent& event) {
    int sel = m_type_choice->GetSelection();

    m_returned_params = ConfigTypes.at(sel).get_params(m_inputs);

    event.Skip(); // Allow the dialog to close normally
}