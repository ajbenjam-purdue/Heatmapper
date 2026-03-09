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
    choices.Add("Conduction: Rectangular Cross Section");
    choices.Add("Conduction: Circular Cross Section");
    choices.Add("Conduction: Annulus Cross Section");
    choices.Add("Conduction: Radial (Cylindrical)");
    choices.Add("Conduction: Spherical");
    choices.Add("Conduction: Contact Resistance");
    choices.Add("Conduction: Known Area Cross Section");
    choices.Add("Shape Factor: Cylinder in Medium to Surface");
    choices.Add("Shape Factor: Sphere in Medium to Surface");
    choices.Add("Shape Factor: Two Parallel Cylinders in Medium");
    choices.Add("Shape Factor: Vertical Cylinder in Medium to Surface");
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

double EdgeConfigDialog::GetVal(const std::string& key)
{
    auto it = m_inputs.find(key);
    if (it == m_inputs.end()) return 0.0;

    double v = 0.0;
    it->second->GetValue().ToDouble(&v);
    return v;
}

void EdgeConfigDialog::ValidateInputs()
{
    int sel = m_type_choice->GetSelection();
    m_warning_text->SetLabel("");

    if (sel == 7)
    { // Cylinder in Medium to Surface
        double L = GetVal("L");
        double D = GetVal("D");
        double z = GetVal("z");
        
        if (L < 1.5 * D)
        {
            m_warning_text->SetLabel(
                "Warning: Shape factor only valid for L ≫ D.");
        }
    }
    else if (sel == 8)
    { // Sphere in Medium to Surface
        double D = GetVal("D");
        double z = GetVal("z");
        
        if (z <= D / 2.0)
        {
            m_warning_text->SetLabel(
                "Warning: Shape factor only valid for z > D/2.");
        }
    }
    else if (sel == 9)
    { // Two Cylinders in Medium
        double L = GetVal("L");
        double D1 = GetVal("D1");
        double D2 = GetVal("D2");
        double w = GetVal("w");
        
        if ((L < 1.5 * D1) || (L < 1.5 * D2) || (L < 1.5 * w))
        {
            m_warning_text->SetLabel(
                "Warning: Shape factor only valid for L ≫ D1, D2 and L ≫ w.");
        }
    }
    else if (sel == 10)
    { // Vertical Cylinder in Medium to Surface
        double L = GetVal("L");
        double D = GetVal("D");

        if (L < D * 1.5)
        {
            m_warning_text->SetLabel(
                "Warning: Shape factor only valid for L ≫ D.");
        }
    }

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

    wxString resDir = wxStandardPaths::Get().GetResourcesDir();
    wxString sep = wxFileName::GetPathSeparator();
    wxString svg_path = resDir + sep + "assets" + sep + "resistances" + sep;

    // Add the Material Dropdown & custom Override
    if (selection != 5)
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

    if (selection == 0) { // Planar Conduction rectangular
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_rectangular.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:", "L");
        AddInputRow("Width (W) [m]:", "W");
        AddInputRow("Height (H) [m]:", "H");
    } 
    else if (selection == 1) { // Planar Conduction circular
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_circular_cross_section.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:", "L");
        AddInputRow("Diameter (D) [m]:", "D");
    }
    else if (selection == 2) { // Planar Conduction annulus
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_donut_cross_section.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:", "L");
        AddInputRow("Inner Diameter (D1) [m]:", "D1");
        AddInputRow("Outer Diameter (D2) [m]:", "D2");
    }
    else if (selection == 3) { // Radial Conduction
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_radial.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:", "L");
        AddInputRow("Inner Diameter (D1) [m]:", "D1");
        AddInputRow("Outer Diameter (D2) [m]:", "D2");
    }
    else if (selection == 4) { // Spherical Conduction
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_spherical.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Inner Diameter (D1) [m]:", "D1");
        AddInputRow("Outer Diameter (D2) [m]:", "D2");
    }
    else if (selection == 5) { // Conduction Contact Resistance
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_contact_resistance.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Contact Resistance (R'') [m2-K/W]:", "R''");
        AddInputRow("Area (A) [m2]:", "A");
    }
    else if (selection == 6) { // Conduction area (unknown shape)
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_area.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:", "L");
        AddInputRow("Area (A) [m2]:", "A");
    }
    else if (selection == 7) { // Cylinder in Medium to Surface
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_sf_buried_isothermal_cylinder.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:", "L");
        AddInputRow("Cylinder Depth (z) [m]:", "z");
        AddInputRow("Cylinder Diameter (D) [m]:", "D");
    }
    else if (selection == 8) { // Sphere in Medium to Surface
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_sf_buried_isothermal_sphere.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Sphere Depth (z) [m]:", "z");
        AddInputRow("Sphere Diameter (D) [m]:", "D");
    }
    else if (selection == 9) { // Two Cylinders in Medium
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_sf_two_cylinders.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]", "L");
        AddInputRow("Diameter 1 (D1) [m]", "D1");
        AddInputRow("Diameter 2 (D2) [m]", "D2");
        AddInputRow("Center-Center Separation (w) [m]", "w");
    }
    else if (selection == 10) { // Vertical Cylinder in Medium to Surface
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_sf_vertical_cylinder.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:", "L");
        AddInputRow("Cylinder Diameter (D) [m]:", "D");
    }
    
    // Only show for not CR
    if (selection != 5)
    {
        wxBoxSizer* k_row = new wxBoxSizer(wxHORIZONTAL);
        k_row->Add(new wxStaticText(this, wxID_ANY, "Thermal Cond. (k) [W/mK]:"), 1, wxALIGN_CENTER_VERTICAL);
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
    // std::vector<double> vals;
    // for (auto* ctrl : m_inputs) {
    //     double v = 0.0;
    //     ctrl->GetValue().ToDouble(&v);
    //     vals.push_back(v);
    // }

    // Just read directly with new helper

    // Actual resistance calculation
    if (sel == 0) 
    {
        // Rectangular: R = L / (k * W * H)
        double L = GetVal("L");
        double W = GetVal("W");
        double H = GetVal("H");
        double k = GetVal("k");

        m_calculated_res = L / std::max(k * W * H, 1e-10);
    }
    else if (sel == 1)
    {
        // Circular: R = 4L / (k * pi * d^2)
        double L = GetVal("L");
        double D = GetVal("D");
        double k = GetVal("k");

        m_calculated_res = 4.0 * L / std::max(k * M_PI * D * D, 1e-10);
    }
    else if (sel == 2)
    {
        // Annulus: R = 4L / (k * pi * (d2^2 - d1^2))
        double L = GetVal("L");
        double D1 = GetVal("D1");
        double D2 = GetVal("D2");
        double k = GetVal("k");

        double D_outer = std::max(D1, D2);
        double D_inner = std::min(D1, D2);

        m_calculated_res = 4.0 * L /
            std::max(k * M_PI * (D_outer * D_outer - D_inner * D_inner), 1e-10);
    }
    else if (sel == 3)
    {
        // Radial (Cylinder): R = ln(d2/d1) / (2*pi*k*L)
        double D1 = GetVal("D1");
        double D2 = GetVal("D2");
        double L = GetVal("L");
        double k = GetVal("k");

        double D_outer = std::max(D1, D2);
        double D_inner = std::min(D1, D2);

        m_calculated_res = std::log(D_outer / D_inner) /
            std::max(2.0 * M_PI * k * L, 1e-10);
    }
    else if (sel == 4)
    {
        // Spherical: R = (1/d1 - 1/d2) / (2*pi*k)
        double D1 = GetVal("D1");
        double D2 = GetVal("D2");
        double k = GetVal("k");

        double D_outer = std::max(D1, D2);
        double D_inner = std::min(D1, D2);

        m_calculated_res =
            (1.0 / D_inner - 1.0 / D_outer) /
            std::max(2.0 * M_PI * k, 1e-10);
    }
    else if (sel == 5)
    {
        // Contact Resistance: R = R'' / A
        double Rpp = GetVal("Rpp");
        double A = GetVal("A");

        m_calculated_res = Rpp / std::max(A, 1e-10);
    }
    else if (sel == 6)
    {
        // Known Area: R = L / (k * A)
        double L = GetVal("L");
        double A = GetVal("A");
        double k = GetVal("k");

        m_calculated_res = L / std::max(k * A, 1e-10);
    }
    else if (sel == 7) 
    { // Cylinder in Medium to Surface: S = 2 pi L / ln(4z/D)
        double D = GetVal("D");
        double z = GetVal("z");
        double L = GetVal("L");
        double S = (2.0 * M_PI * L) / std::log(4.0 * z / D);
        
        m_calculated_res = 1.0 / (S * GetVal("k"));
    }
    else if (sel == 8) 
    { // Sphere in Medium to Surface: S = 2 pi D / (1 - D/4z)
        double D = GetVal("D");
        double z = GetVal("z");
        double S = (2.0 * M_PI * D) / (1 - D / (4.0 * z));
        
        m_calculated_res = 1.0 / (S * GetVal("k"));
    }
    else if (sel == 9) 
    { // Two Cylinders in Medium: I'm not writing it twice
        double L = GetVal("L");
        double D1 = GetVal("D1");
        double D2 = GetVal("D2");
        double w = GetVal("w");
        double S = 2.0 * M_PI * L / std::acosh((4.0 * w * w - D1 * D1 - D2 * D2) / (2.0 * D1 * D2));
        
        m_calculated_res = 1.0 / (S * GetVal("k"));
    }
    else if (sel == 10) 
    { // Vertical Cylinder in Medium to Surface: S = (2 pi L) / ln(4L/D)
        double D = GetVal("D");
        double L = GetVal("L");
        double S = 2.0 * M_PI * L / std::log(4.0 * L / D);
        
        m_calculated_res = 1.0 / (S * GetVal("k"));
    }

    event.Skip(); // Allow the dialog to close normally
}