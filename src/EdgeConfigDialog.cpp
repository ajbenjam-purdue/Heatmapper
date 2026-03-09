#include "EdgeConfigDialog.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

// Assign specific IDs for the two types of selectors
enum {
    ID_TypeChoice = 1001,
    ID_MatChoice = 1002
};

wxBEGIN_EVENT_TABLE(EdgeConfigDialog, wxDialog)
    EVT_CHOICE(ID_MatChoice, EdgeConfigDialog::OnMaterialChange)
    EVT_CHOICE(wxID_ANY, EdgeConfigDialog::OnTypeChange)
    EVT_BUTTON(wxID_OK, EdgeConfigDialog::OnOK)
wxEND_EVENT_TABLE()

EdgeConfigDialog::EdgeConfigDialog(wxWindow* parent, const MaterialLibrary& mat_lib) 
    : wxDialog(parent, wxID_ANY, "Configure Edge Resistance", wxDefaultPosition, wxSize(450, 500)),
      m_mat_lib(mat_lib)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Dropdown menu
    wxArrayString choices;
    choices.Add("Conduction: Rectangular");
    choices.Add("Conduction: Circular");
    choices.Add("Conduction: Annulus");
    choices.Add("Conduction: Radial (Cylindrical)");
    choices.Add("Conduction: Known Area");
    m_type_choice = new wxChoice(this, ID_TypeChoice, wxDefaultPosition, wxDefaultSize, choices);
    m_type_choice->SetSelection(0);
    main_sizer->Add(m_type_choice, 0, wxEXPAND | wxALL, 10);

    // SVG Image Placeholder
    m_svg_display = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    main_sizer->Add(m_svg_display, 0, wxALIGN_CENTER | wxALL, 10);

    // Dynamic Sizer for text boxes
    m_dynamic_input_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(m_dynamic_input_sizer, 1, wxEXPAND | wxALL, 10);

    // Buttons
    main_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(main_sizer);

    // Build the initial state
    BuildInputs(0); 
}

void EdgeConfigDialog::BuildInputs(int selection) {
    m_dynamic_input_sizer->Clear(true);
    m_current_inputs.clear();

    wxString resDir = wxStandardPaths::Get().GetResourcesDir();
    wxString sep = wxFileName::GetPathSeparator();
    wxString svg_path = resDir + sep + "assets" + sep + "resistances" + sep;

    auto AddInputRow = [&](const wxString& label) {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, label), 1, wxALIGN_CENTER_VERTICAL);
        wxTextCtrl* input = new wxTextCtrl(this, wxID_ANY, "1.0");
        m_current_inputs.push_back(input);
        row->Add(input, 1, wxEXPAND);
        m_dynamic_input_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 5);
    };

    // Add the Material Dropdown & custom Override
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

    if (selection == 0) { // Planar Conduction rectangular
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_rectangular.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Width (W) [m]:");
        AddInputRow("Height (H) [m]:");
    } 
    else if (selection == 1) { // Planar Conduction circular
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_circular_cross_section.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Diameter (D) [m]:");
    }
    else if (selection == 2) { // Planar Conduction annulus
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_donut_cross_section.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Inner Diameter (D1) [m]:");
        AddInputRow("Outer Diameter (D2) [m]:");
    }
    else if (selection == 3) { // Radial Conduction
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_radial.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Inner Diameter (D1) [m]:");
        AddInputRow("Outer Diameter (D2) [m]:");
    }// TODO: CREATE MEDIA + ADD SPHERICAL SHELL
    else if (selection == 4) { // Conduction area (unknown shape)
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_area.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Area [m2]:");
    }
    
    wxBoxSizer* k_row = new wxBoxSizer(wxHORIZONTAL);
    k_row->Add(new wxStaticText(this, wxID_ANY, "Thermal Cond. (k) [W/mK]:"), 1, wxALIGN_CENTER_VERTICAL);
    m_k_input = new wxTextCtrl(this, wxID_ANY, "");
    m_current_inputs.push_back(m_k_input); // Push to array so math logic still finds it at the end
    k_row->Add(m_k_input, 1, wxEXPAND);
    m_dynamic_input_sizer->Add(k_row, 0, wxEXPAND | wxBOTTOM, 5);

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
    if (sel >= 0 && sel < m_mat_lib.materials.size()) {
        double k = m_mat_lib.materials[sel].thermal_conductivity;
        m_k_input->Disable(); // Disable the input
        m_k_input->SetValue(wxString::Format("%.2f", k));
    } 
    else {
        // "Custom..." selected
        m_k_input->Enable(); // Unlock it
    }
}

void EdgeConfigDialog::OnOK(wxCommandEvent& event) {
    int sel = m_type_choice->GetSelection();
    
    // Read all strings from the dynamic inputs and convert to doubles
    std::vector<double> vals;
    for (auto* ctrl : m_current_inputs) {
        double v = 0.0;
        ctrl->GetValue().ToDouble(&v);
        vals.push_back(v);
    }

    // Actual res calc
    if (sel == 0 && vals.size() == 4) {
        // Rectangular: R = L / (k * W * H)
        m_calculated_res = vals[0] / std::max(vals[3] * vals[1] * vals[2], 1e-10); // I genuinely think I get this wrong every time I do it. crazy!
    } 
    else if (sel == 1 && vals.size() == 3) {
        // Circular: R = 4L / (k * pi * d^2)
        m_calculated_res = 4.0 * vals[0] / std::max(vals[2] * M_PI * vals[1] * vals[1], 1e-10);
    }
    else if (sel == 2 && vals.size() == 4) {
        // Annulus: R = 4L / (k * pi * (d_2^2-d_1^2))
        double D_2 = std::max(vals[1], vals[2]);
        double D_1 = std::min(vals[1], vals[2]);
        m_calculated_res = 4.0 * vals[0] / std::max(vals[3] * M_PI * (D_2 * D_2 - D_1 * D_1), 1e-10);
    }
    else if (sel == 3 && vals.size() == 4) {
        // Radial (Cylinder): R = ln(d2/d1) / (2 * pi * k * L)
        m_calculated_res = std::log(vals[1] / vals[0]) / std::max(2.0 * M_PI * vals[3] * vals[2], 1e-10);
    }
    else if (sel == 4 && vals.size() == 3) {
        // Area: R = L / (k * A)
        m_calculated_res = vals[0] / std::max(vals[1] * vals[2], 1e-10);
    }

    event.Skip(); // Allow the dialog to close normally
}