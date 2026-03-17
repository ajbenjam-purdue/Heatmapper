#include "PrefsPage.h"

wxBEGIN_EVENT_TABLE(GeneralPrefsPanel, wxPanel)
    EVT_CHECKBOX(wxID_ANY, GeneralPrefsPanel::OnPrefChanged)
    EVT_TEXT(wxID_ANY, GeneralPrefsPanel::OnPrefChanged)
    EVT_SPINCTRL(wxID_ANY, GeneralPrefsPanel::OnSpinPrefChanged)
    EVT_SPINCTRLDOUBLE(wxID_ANY, GeneralPrefsPanel::OnSpinDoublePrefChanged)
wxEND_EVENT_TABLE()

GeneralPrefsPanel::GeneralPrefsPanel(wxWindow* parent) : wxPanel(parent) {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* save_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Autosave");
    
    // Autosaving
    wxBoxSizer* row1 = new wxBoxSizer(wxHORIZONTAL);
    m_autosave_enable = new wxCheckBox(this, wxID_ANY, "Enable Autosave");
    row1->Add(m_autosave_enable, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    
    row1->Add(new wxStaticText(this, wxID_ANY, "Interval (mins):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_autosave_mins = new wxSpinCtrl(this, wxID_ANY);
    m_autosave_mins->SetRange(1, 60);
    row1->Add(m_autosave_mins, 0, wxALIGN_CENTER_VERTICAL);
    
    save_sizer->Add(row1, 0, wxALL | wxEXPAND, 5);
    main_sizer->Add(save_sizer, 0, wxEXPAND | wxALL, 10);

    // UI Settings
    wxStaticBoxSizer* ui_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Interface");
    wxBoxSizer* row2 = new wxBoxSizer(wxHORIZONTAL);
    row2->Add(new wxStaticText(this, wxID_ANY, "Node Draw Radius (px):"), 1, wxALIGN_CENTER_VERTICAL);
    m_node_size = new wxSpinCtrl(this, wxID_ANY);
    m_node_size->SetRange(4, 16);
    row2->Add(m_node_size, 1, wxEXPAND);
    ui_sizer->Add(row2, 0, wxALL | wxEXPAND, 5);
    main_sizer->Add(ui_sizer, 0, wxEXPAND | wxALL, 10);

    // Simulation Settings
    wxStaticBoxSizer* sim_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Simulation Defaults");
    
    wxBoxSizer* row3 = new wxBoxSizer(wxHORIZONTAL);
    row3->Add(new wxStaticText(this, wxID_ANY, "Transient Time Step (s):"), 1, wxALIGN_CENTER_VERTICAL);
    
    wxFloatingPointValidator<double> dt_validator(4); // 4 decimal places
    dt_validator.SetRange(1e-4, 0.1);
    
    m_default_dt = new wxTextCtrl(this, wxID_ANY, "0.01", wxDefaultPosition, wxDefaultSize, 0, dt_validator);
    row3->Add(m_default_dt, 1, wxEXPAND);
    sim_sizer->Add(row3, 0, wxALL | wxEXPAND, 5);

    // Workspace Defaults
    wxBoxSizer* row4 = new wxBoxSizer(wxHORIZONTAL);
    row4->Add(new wxStaticText(this, wxID_ANY, "Ambient Node Temp (°C):"), 1, wxALIGN_CENTER_VERTICAL);
    
    // TODO: Fix this scallywag
    m_default_ambient = new wxSpinCtrlDouble(this, wxID_ANY);
    m_default_ambient->SetRange(-273.15, 1250.0);
    m_default_ambient->SetIncrement(0.5);
    m_default_ambient->SetDigits(1);
    
    row4->Add(m_default_ambient, 1, wxEXPAND);
    sim_sizer->Add(row4, 0, wxALL | wxEXPAND, 5);

    main_sizer->Add(sim_sizer, 0, wxEXPAND | wxALL, 10);

    SetSizerAndFit(main_sizer);
}

bool GeneralPrefsPanel::TransferDataToWindow() {
    m_is_loading = true;

    wxConfigBase* config = wxConfigBase::Get();
    if (!config)
    {
        m_is_loading = false;
        return false;
    }

    m_autosave_enable->SetValue(config->ReadBool("/Autosave/Enabled", true));
    m_autosave_mins->SetValue(config->ReadLong("/Autosave/Interval", 5));
    m_node_size->SetValue(config->ReadLong("/UI/NodeRadius", 15));
    m_default_dt->SetValue(config->Read("/Sim/DefaultDt", "0.01"));
    double ambient_val = 15.0; 
    config->Read("/Sim/DefaultAmbient", &ambient_val, 15.0); 
    m_default_ambient->SetValue(ambient_val);

    m_is_loading = false;
    return true;
}

bool GeneralPrefsPanel::TransferDataFromWindow() {
    wxConfigBase* config = wxConfigBase::Get();
    if (!config) return false;

    config->Write("/Autosave/Enabled", m_autosave_enable->GetValue());
    config->Write("/Autosave/Interval", m_autosave_mins->GetValue());
    config->Write("/UI/NodeRadius", m_node_size->GetValue());
    config->Write("/Sim/DefaultDt", m_default_dt->GetValue());
    config->Write("/Sim/DefaultAmbient", m_default_ambient->GetValue());
    
    config->Flush(); 
    return true;
}

// Real-time hook so MacOS applies settings instantly without an "OK" button (Thanks, Gemini!)
void GeneralPrefsPanel::OnPrefChanged(wxCommandEvent& event) {
    if (m_is_loading) {
        event.Skip();
        return;
    }
    
    TransferDataFromWindow();
    event.Skip();
}
void GeneralPrefsPanel::OnSpinPrefChanged(wxSpinEvent& event) {
    if (m_is_loading) {
        event.Skip();
        return;
    }
    
    TransferDataFromWindow();
    event.Skip();
}
void GeneralPrefsPanel::OnSpinDoublePrefChanged(wxSpinDoubleEvent& event) {
    if (m_is_loading) {
        event.Skip();
        return;
    }
    
    TransferDataFromWindow();
    event.Skip();
}