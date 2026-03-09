#include "EdgeConfigDialog.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

wxBEGIN_EVENT_TABLE(EdgeConfigDialog, wxDialog)
    EVT_CHOICE(wxID_ANY, EdgeConfigDialog::OnTypeChange)
    EVT_BUTTON(wxID_OK, EdgeConfigDialog::OnOK)
wxEND_EVENT_TABLE()

EdgeConfigDialog::EdgeConfigDialog(wxWindow* parent) 
    : wxDialog(parent, wxID_ANY, "Configure Edge Resistance", wxDefaultPosition, wxSize(450, 500)) 
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    // Dropdown menu
    wxArrayString choices;
    choices.Add("Conduction: Rectangular");
    choices.Add("Conduction: Circular");
    choices.Add("Conduction: Annulus");
    choices.Add("Conduction: Radial (Cylindrical)");
    choices.Add("Conduction: Known Area");
    m_type_choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
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
    // Clear the old text boxes
    m_dynamic_input_sizer->Clear(true);
    m_current_inputs.clear();

    wxString resDir = wxStandardPaths::Get().GetResourcesDir();
    wxString sep = wxFileName::GetPathSeparator();
    wxString svg_path = resDir + sep + "assets" + sep + "resistances" + sep;

    // Helper lambda to make adding rows easy
    auto AddInputRow = [&](const wxString& label) {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, label), 1, wxALIGN_CENTER_VERTICAL);
        wxTextCtrl* input = new wxTextCtrl(this, wxID_ANY, "1.0");
        m_current_inputs.push_back(input);
        row->Add(input, 1, wxEXPAND);
        m_dynamic_input_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 5);
    };

    if (selection == 0) { // Conduction rectangular
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_rectangular.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Width (W) [m]:");
        AddInputRow("Height (H) [m]:");
        AddInputRow("Thermal Cond. (k) [W/mK]:");
    } 
    else if (selection == 1) { // Conduction circular
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_circular_cross_section.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Diameter (D) [m]:");
        AddInputRow("Thermal Cond. (k) [W/mK]:");
    }
    else if (selection == 2) { // Conduction annulus
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_donut_cross_section.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Inner Diameter (D1) [m]:");
        AddInputRow("Outer Diameter (D2) [m]:");
        AddInputRow("Thermal Cond. (k) [W/mK]:");
    }
    else if (selection == 3) { // Conduction radial
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_radial.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Inner Diameter (D1) [m]:");
        AddInputRow("Outer Diameter (D2) [m]:");
        AddInputRow("Thermal Cond. (k) [W/mK]:");
    }// TODO: CREATE MEDIA + ADD SPHERICAL SHELL
    else if (selection == 4) { // Conduction area (unknown shape)
        m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "conduction_area.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Length (L) [m]:");
        AddInputRow("Area [m2]:");
        AddInputRow("Thermal Cond. (k) [W/mK]:");
    }

    Layout(); // Redraw the window with the new fields
}

void EdgeConfigDialog::OnTypeChange(wxCommandEvent& event) {
    BuildInputs(m_type_choice->GetSelection());
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
        m_calculated_res = vals[0] / std::min(vals[3] * vals[1] * vals[2], 1e-10);
    } 
    else if (sel == 1 && vals.size() == 3) {
        // Circular: R = 4L / (k * pi * d^2)
        m_calculated_res = 4.0 * vals[0] / (vals[2] * M_PI * vals[1] * vals[1]);
    }
    else if (sel == 2 && vals.size() == 4) {
        // Annulus: R = 4L / (k * pi * (d_2^2-d_1^2))
        double D_2 = std::max(vals[1], vals[2]);
        double D_1 = std::min(vals[1], vals[2]);
        m_calculated_res = 4.0 * vals[0] / (vals[3] * M_PI * (D_2 * D_2 - D_1 * D_1));
    }
    else if (sel == 3 && vals.size() == 4) {
        // Radial (Cylinder): R = ln(d2/d1) / (2 * pi * k * L)
        m_calculated_res = std::log(vals[1] / vals[0]) / (2.0 * M_PI * vals[3] * vals[2]);
    }
    else if (sel == 4 && vals.size() == 3) {
        // Area: R = L / (k * A)
        m_calculated_res = vals[0] / (vals[1] * vals[2]);
    }

    event.Skip(); // Allow the dialog to close normally
}