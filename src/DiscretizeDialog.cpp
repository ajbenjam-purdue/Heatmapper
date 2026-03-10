#include "DiscretizeDialog.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

enum {
    ID_TypeChoice = 1001
};

wxBEGIN_EVENT_TABLE(DiscretizeDialog, wxDialog)
    EVT_CHOICE(ID_TypeChoice, DiscretizeDialog::OnTypeChange)
wxEND_EVENT_TABLE()

DiscretizeDialog::DiscretizeDialog(wxWindow* parent, const MaterialLibrary& mat_lib) 
    : wxDialog(parent, wxID_ANY, "Discretize Node", wxDefaultPosition, wxSize(450, 400)),
      m_mat_lib(mat_lib)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString choices;
    choices.Add("N-Node Chain");
    choices.Add("N-Node Centralized Hub");
    choices.Add("NxM Comb Graph");
    m_type_choice = new wxChoice(this, ID_TypeChoice, wxDefaultPosition, wxDefaultSize, choices);
    m_type_choice->SetSelection(0);
    main_sizer->Add(m_type_choice, 0, wxEXPAND | wxALL, 10);

    m_svg_display = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    main_sizer->Add(m_svg_display, 0, wxALIGN_CENTER | wxALL, 10);

    m_dynamic_input_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(m_dynamic_input_sizer, 1, wxEXPAND | wxALL, 10);

    main_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 10);
    
    SetSizer(main_sizer);
    BuildInputs(0);
}

wxTextCtrl* DiscretizeDialog::AddInputRow(const wxString& label, const std::string& key, const wxString& default_val) {
    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(new wxStaticText(this, wxID_ANY, label), 1, wxALIGN_CENTER_VERTICAL);
    wxTextCtrl* input = new wxTextCtrl(this, wxID_ANY, default_val);
    row->Add(input, 1, wxEXPAND);
    m_dynamic_input_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 5);
    m_inputs[key] = input;
    return input;
}

double DiscretizeDialog::GetVal(const std::string& key) const {
    auto it = m_inputs.find(key);
    if (it == m_inputs.end()) return 0.0;
    double v = 0.0;
    it->second->GetValue().ToDouble(&v);
    return v;
}

int DiscretizeDialog::GetIntVal(const std::string& key) const {
    auto it = m_inputs.find(key);
    if (it == m_inputs.end()) return 0;
    long v = 0;
    it->second->GetValue().ToLong(&v);
    return static_cast<int>(v);
}

void DiscretizeDialog::BuildInputs(int selection) {
    m_dynamic_input_sizer->Clear(true);
    m_inputs.clear();

    wxString resDir = wxStandardPaths::Get().GetResourcesDir();
    wxString sep = wxFileName::GetPathSeparator();
    wxString svg_path = resDir + sep + "assets" + sep + "discretize" + sep;

    // Haven't yet created the SVGs for this section
    if (selection == 0) {
        // m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "discretize_chain.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Number of Nodes (N):", "N", "3");
    } 
    else if (selection == 1) {
        // m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "discretize_hub.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Number of Spoke Nodes (N):", "N", "4");
    }
    else if (selection == 2) {
        // m_svg_display->SetBitmap(wxBitmapBundle::FromSVGFile(svg_path + "discretize_comb.svg", wxSize(200, 150)).GetBitmapFor(this));
        AddInputRow("Number of Teeth (N):", "N", "3");
        AddInputRow("Nodes per Tooth (M):", "M", "2");
    }

    AddInputRow("Internal Resistance between nodes (R) [K/W]:", "R", "1.0");

    Layout();
}

void DiscretizeDialog::OnTypeChange(wxCommandEvent& event) {
    BuildInputs(m_type_choice->GetSelection());
}

int DiscretizeDialog::GetDiscretizationType() const { return m_type_choice->GetSelection(); }
int DiscretizeDialog::GetN() const { return GetIntVal("N"); }
int DiscretizeDialog::GetM() const { return GetIntVal("M"); }
double DiscretizeDialog::GetInternalResistance() const { return GetVal("R"); }