#include "TransientDialog.h"

enum
{
    ID_USE_AUTO = wxID_HIGHEST + 1,
    ID_BROWSE_CSV
};

wxBEGIN_EVENT_TABLE(TransientDialog, wxDialog)
    EVT_CHECKBOX(ID_USE_AUTO, TransientDialog::OnCheck)
    EVT_BUTTON(ID_BROWSE_CSV, TransientDialog::OnBrowseCSV)
wxEND_EVENT_TABLE()

TransientDialog::TransientDialog(wxWindow* parent, double dt, bool save_csv, bool save_compressed)
    : wxDialog(parent, wxID_ANY, "Transient Solver Options", wxDefaultPosition, wxDefaultSize)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    m_checkbox = new wxCheckBox(this, ID_USE_AUTO, "Stop on steady state");

    m_end_criteria_input = new wxSpinCtrlDouble(this, wxID_ANY);
    m_end_criteria_input->SetRange(0.1, 3600.0);
    m_end_criteria_input->SetIncrement(0.1);

    m_dt_input = new wxSpinCtrlDouble(this, wxID_ANY);
    m_dt_input->SetRange(1e-6, 1);
    m_dt_input->SetIncrement(0.001);
    m_dt_input->SetValue(dt);

    mainSizer->Add(m_checkbox, 0, wxALL, 8);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, "End Criteria (s):"), 0, wxLEFT | wxRIGHT, 8);
    mainSizer->Add(m_end_criteria_input, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Time Step dt (s):"), 0, wxLEFT | wxRIGHT, 8);
    mainSizer->Add(m_dt_input, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    wxBoxSizer* csvOptionSizer = new wxBoxSizer(wxHORIZONTAL);
    m_save_csv_checkbox = new wxCheckBox(this, wxID_ANY, "Save CSV");
    m_save_csv_checkbox->SetValue(save_csv);
    m_save_compressed_checkbox = new wxCheckBox(this, wxID_ANY, "Save Compressed (bz2)");
    m_save_compressed_checkbox->SetValue(save_compressed);

    csvOptionSizer->Add(m_save_csv_checkbox, 0, wxRIGHT, 10);
    csvOptionSizer->Add(m_save_compressed_checkbox, 0);
    mainSizer->Add(csvOptionSizer, 0, wxLEFT|wxRIGHT|wxTOP, 8);

    wxBoxSizer* csvSizer = new wxBoxSizer(wxHORIZONTAL);

    m_csv_path = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    m_browse_button = new wxButton(this, ID_BROWSE_CSV, "Browse...");

    csvSizer->Add(m_csv_path, 1, wxEXPAND | wxRIGHT, 5);
    csvSizer->Add(m_browse_button, 0);
    m_csv_path->SetValue(wxStandardPaths::Get().GetDocumentsDir() +  wxFileName::GetPathSeparator() + "transient_output.csv");
    m_csv_path->SetMinSize(wxSize(350, -1));

    mainSizer->Add(new wxStaticText(this, wxID_ANY, "CSV Save Path:"), 0, wxLEFT|wxRIGHT|wxTOP, 8);
    mainSizer->Add(csvSizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 8);

    mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);

    SetSizerAndFit(mainSizer);

    // initialize enabled state
    m_end_criteria_input->Enable(!m_checkbox->GetValue());
}

void TransientDialog::OnCheck(wxCommandEvent& event)
{
    m_end_criteria_input->Enable(!m_checkbox->GetValue());
}

bool TransientDialog::GetSteadyStateEnd() const
{
    return m_checkbox->GetValue();
}

double TransientDialog::GetEndCriteria() const
{
    return m_end_criteria_input->GetValue();
}

double TransientDialog::GetTimeStep() const
{
    return m_dt_input->GetValue();
}

bool TransientDialog::GetSaveCSV() const
{
    return m_save_csv_checkbox->GetValue();
}

bool TransientDialog::GetSaveCompressedCSV() const
{
    return m_save_compressed_checkbox->GetValue();
}

void TransientDialog::OnBrowseCSV(wxCommandEvent& event)
{
    wxFileDialog fileDialog(
        this,
        "Choose CSV Save Location",
        "",
        "",
        "CSV files (*.csv)|*.csv",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (fileDialog.ShowModal() == wxID_OK)
    {
        m_csv_path->SetValue(fileDialog.GetPath());
    }
}

std::string TransientDialog::GetSaveFilePath() const
{
    return m_csv_path->GetValue().ToStdString();
}