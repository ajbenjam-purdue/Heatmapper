#include "DiscretizeDialog.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

enum {
    ID_TypeChoice = 1001,
    ID_ConfigR1 = 1002, // Button for R1
    ID_ConfigR2 = 1003  // Button for R2
};

wxBEGIN_EVENT_TABLE(DiscretizeDialog, wxDialog)
    EVT_CHOICE(ID_TypeChoice, DiscretizeDialog::OnTypeChange)
    EVT_BUTTON(ID_ConfigR1, DiscretizeDialog::OnConfigR1)
    EVT_BUTTON(ID_ConfigR2, DiscretizeDialog::OnConfigR2)
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

// NEW HELPER: Creates a row with a Read-Only text box and a "Configure..." button
void DiscretizeDialog::AddResistanceConfigRow(const wxString& label, int button_id, wxTextCtrl** out_display, double current_val) {
    wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(new wxStaticText(this, wxID_ANY, label), 1, wxALIGN_CENTER_VERTICAL);
    
    // Read-only display box
    *out_display = new wxTextCtrl(this, wxID_ANY, wxString::Format("%.4f", current_val), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    row->Add(*out_display, 1, wxEXPAND | wxRIGHT, 5);
    
    // Configure Button
    wxButton* config_btn = new wxButton(this, button_id, "Configure...");
    row->Add(config_btn, 0, wxEXPAND);
    
    m_dynamic_input_sizer->Add(row, 0, wxEXPAND | wxBOTTOM, 5);
}

void DiscretizeDialog::BuildInputs(int selection) {
    m_dynamic_input_sizer->Clear(true);
    m_inputs.clear();

    wxString resDir = wxStandardPaths::Get().GetResourcesDir();
    wxString sep = wxFileName::GetPathSeparator();
    wxString svg_path = resDir + sep + "assets" + sep + "discretize" + sep;

    if (selection == 0) {
        AddInputRow("Number of Nodes (N):", "N", "3");
        AddResistanceConfigRow("Internal Resistance (R) [K/W]:", ID_ConfigR1, &m_r1_display, m_res_1);
    } 
    else if (selection == 1) {
        AddInputRow("Number of Spoke Nodes (N):", "N", "4");
        AddResistanceConfigRow("Internal Resistance (R) [K/W]:", ID_ConfigR1, &m_r1_display, m_res_1);
    }
    else if (selection == 2) {
        AddInputRow("Number of Teeth (N):", "N", "3");
        AddInputRow("Nodes per Tooth (M):", "M", "2");

        // The Comb gets two config rows.
        // I don't love how this feels semantically so might refactor later
        AddResistanceConfigRow("Spine Resistance (R1) [K/W]:", ID_ConfigR1, &m_r1_display, m_res_1);
        AddResistanceConfigRow("Tooth Resistance (R2) [K/W]:", ID_ConfigR2, &m_r2_display, m_res_2);
    }

    Layout();
}

void DiscretizeDialog::OnTypeChange(wxCommandEvent& event) {
    BuildInputs(m_type_choice->GetSelection());
}

// Launch the Edge Dialog and capture the result for R1!
void DiscretizeDialog::OnConfigR1(wxCommandEvent& event) {
    EdgeConfigDialog edge_dialog(this, m_mat_lib);
    if (edge_dialog.ShowModal() == wxID_OK) {
        m_res_1 = edge_dialog.GetCalculatedResistance();
        if (m_r1_display) m_r1_display->SetValue(wxString::Format("%.4f", m_res_1));
    }
}

// Launch the Edge Dialog and capture the result for R2!
void DiscretizeDialog::OnConfigR2(wxCommandEvent& event) {
    EdgeConfigDialog edge_dialog(this, m_mat_lib);
    if (edge_dialog.ShowModal() == wxID_OK) {
        m_res_2 = edge_dialog.GetCalculatedResistance();
        if (m_r2_display) m_r2_display->SetValue(wxString::Format("%.4f", m_res_2));
    }
}

int DiscretizeDialog::GetDiscretizationType() const { return m_type_choice->GetSelection(); }
int DiscretizeDialog::GetN() const { return GetIntVal("N"); }
int DiscretizeDialog::GetM() const { return GetIntVal("M"); }
double DiscretizeDialog::GetInternalResistance() const { return GetVal("R"); }