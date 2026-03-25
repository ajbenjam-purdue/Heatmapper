#include "PrefsPage.h"

wxBEGIN_EVENT_TABLE(GeneralPrefsPanel, wxPanel)
    EVT_CHECKBOX(wxID_ANY, GeneralPrefsPanel::OnPrefChanged)
    EVT_TEXT(wxID_ANY, GeneralPrefsPanel::OnPrefChanged)
    EVT_SPINCTRL(wxID_ANY, GeneralPrefsPanel::OnSpinPrefChanged)
    EVT_SPINCTRLDOUBLE(wxID_ANY, GeneralPrefsPanel::OnSpinDoublePrefChanged)
    EVT_CHOICE(wxID_ANY, GeneralPrefsPanel::OnPrefChanged)
wxEND_EVENT_TABLE()

GeneralPrefsPanel::GeneralPrefsPanel(wxWindow* parent) : wxPanel(parent) {
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* save_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Autosave");
    
    // Autosaving
    wxBoxSizer* row1 = new wxBoxSizer(wxHORIZONTAL);
    m_autosave_enable = new wxCheckBox(save_sizer->GetStaticBox(), wxID_ANY, "Enable Autosave");
    m_autosave_mins = new wxSpinCtrl(save_sizer->GetStaticBox(), wxID_ANY);
    m_autosave_mins->SetRange(1, 60);
    row1->Add(m_autosave_enable, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    row1->Add(new wxStaticText(save_sizer->GetStaticBox(), wxID_ANY, "Interval (mins):"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    row1->Add(m_autosave_mins, 0, wxALIGN_CENTER_VERTICAL);

    save_sizer->Add(row1, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);

    // UI Settings
    wxStaticBoxSizer* ui_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Interface");
    wxBoxSizer* row2 = new wxBoxSizer(wxHORIZONTAL);
    row2->Add(new wxStaticText(ui_sizer->GetStaticBox(), wxID_ANY, "Node Draw Radius (px):"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    m_node_size = new wxSpinCtrl(ui_sizer->GetStaticBox(), wxID_ANY);
    m_node_size->SetRange(4, 16);
    row2->Add(m_node_size, 1, wxEXPAND);

    wxBoxSizer* row8 = new wxBoxSizer(wxHORIZONTAL);
    row8->Add(new wxStaticText(ui_sizer->GetStaticBox(), wxID_ANY, "Node Color Scheme:"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    wxArrayString unit_choices;
    unit_choices.Add("Viridis"); unit_choices.Add("Plasma"); unit_choices.Add("Magma");
    m_scheme = new wxChoice(ui_sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, unit_choices);
    row8->Add(m_scheme, 1, wxEXPAND);

    wxBoxSizer* row9 = new wxBoxSizer(wxHORIZONTAL);
    row9->Add(new wxStaticText(ui_sizer->GetStaticBox(), wxID_ANY, "Grid Snapping:"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    wxArrayString grid_choices;
    grid_choices.Add("Fine"); grid_choices.Add("Coarse"); grid_choices.Add("None");
    m_grid_snapping = new wxChoice(ui_sizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, grid_choices);
    row9->Add(m_grid_snapping, 1, wxEXPAND);

    ui_sizer->Add(row8, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);
    ui_sizer->Add(row9, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);
    ui_sizer->Add(row2, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);

    // Simulation Settings
    wxStaticBoxSizer* sim_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "Simulation Defaults");
    
    wxBoxSizer* row3 = new wxBoxSizer(wxHORIZONTAL);
    row3->Add(new wxStaticText(sim_sizer->GetStaticBox(), wxID_ANY, "Transient Time Step (s):"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    wxFloatingPointValidator<double> dt_validator(4); // 4 decimal places
    dt_validator.SetRange(1e-4, 0.1);
    m_default_dt = new wxTextCtrl(sim_sizer->GetStaticBox(), wxID_ANY, "0.01", wxDefaultPosition, wxDefaultSize, 0, dt_validator);
    row3->Add(m_default_dt, 1, wxEXPAND);

    wxBoxSizer* row4 = new wxBoxSizer(wxHORIZONTAL);
    row4->Add(new wxStaticText(sim_sizer->GetStaticBox(), wxID_ANY, "Maximum Steady State Iterations:"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    wxIntegerValidator<int> iter_validator;
    iter_validator.SetRange(0, 1000);
    m_ss_iterations_max = new wxTextCtrl(sim_sizer->GetStaticBox(), wxID_ANY, "100", wxDefaultPosition, wxDefaultSize, 0, iter_validator);
    row4->Add(m_ss_iterations_max, 1, wxEXPAND);

    wxBoxSizer* row5 = new wxBoxSizer(wxHORIZONTAL);
    row5->Add(new wxStaticText(sim_sizer->GetStaticBox(), wxID_ANY, "Maximum Steady State Tolerance (C):"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    wxFloatingPointValidator<double> ss_tolerance_max_validator(4); // 4 decimal places
    ss_tolerance_max_validator.SetRange(1e-4, 0.1);
    m_ss_tolerance_max = new wxTextCtrl(sim_sizer->GetStaticBox(), wxID_ANY, "0.01", wxDefaultPosition, wxDefaultSize, 0, ss_tolerance_max_validator);
    row5->Add(m_ss_tolerance_max, 1, wxEXPAND);

    wxBoxSizer* row6 = new wxBoxSizer(wxHORIZONTAL);
    row6->Add(new wxStaticText(sim_sizer->GetStaticBox(), wxID_ANY, "Steady State Relaxation (-):"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    wxFloatingPointValidator<double> ss_relaxation_validator(4); // 4 decimal places
    ss_relaxation_validator.SetRange(1e-2, 1.0);
    m_ss_relaxation = new wxTextCtrl(sim_sizer->GetStaticBox(), wxID_ANY, "0.75", wxDefaultPosition, wxDefaultSize, 0, ss_relaxation_validator);
    row6->Add(m_ss_relaxation, 1, wxEXPAND);

    wxBoxSizer* row7 = new wxBoxSizer(wxHORIZONTAL);
    row7->Add(new wxStaticText(sim_sizer->GetStaticBox(), wxID_ANY, "Ambient Node Temp (C):"), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    m_default_ambient = new wxSpinCtrlDouble(sim_sizer->GetStaticBox(), wxID_ANY);
    m_default_ambient->SetRange(-273.15, 1250.0);
    m_default_ambient->SetIncrement(0.5);
    m_default_ambient->SetDigits(1);
    row7->Add(m_default_ambient, 1, wxEXPAND);

    sim_sizer->Add(row3, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);
    sim_sizer->Add(row4, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);
    sim_sizer->Add(row5, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);
    sim_sizer->Add(row6, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);
    sim_sizer->Add(row7, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 4);

    main_sizer->Add(sim_sizer, 0, wxEXPAND | wxALL, 5);
    main_sizer->Add(save_sizer, 0, wxEXPAND | wxALL, 5);
    main_sizer->Add(ui_sizer, 0, wxEXPAND | wxALL, 5);

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
    m_ss_iterations_max->SetValue(config->Read("/Sim/MaxSSIterations", "100"));
    m_ss_relaxation->SetValue(config->Read("/Sim/SSRelaxation", "0.75"));
    m_ss_tolerance_max->SetValue(config->Read("/Sim/MaxSSTolerance", "0.001"));
    m_default_dt->SetValue(config->Read("/Sim/DefaultDt", "0.01"));
    double ambient_val = 15.0; 
    config->Read("/Sim/DefaultAmbient", &ambient_val, 15.0); 
    m_default_ambient->SetValue(ambient_val);

    // Colors
    wxString saved_unit = config->Read("/UI/NodeScheme", "Celsius");
    int choice_index = m_scheme->FindString(saved_unit);
    if (choice_index != wxNOT_FOUND) {
        m_scheme->SetSelection(choice_index);
    } else {
        m_scheme->SetSelection(0); 
    }

    // Snapping
    wxString saved_grid = config->Read("/UI/GridSnap", "None");
    choice_index = m_grid_snapping->FindString(saved_grid);
    if (choice_index != wxNOT_FOUND) {
        m_grid_snapping->SetSelection(choice_index);
    } else {
        m_grid_snapping->SetSelection(0); 
    }

    m_is_loading = false;
    return true;
}

bool GeneralPrefsPanel::TransferDataFromWindow() {
    wxConfigBase* config = wxConfigBase::Get();
    if (!config) return false;

    config->Write("/Autosave/Enabled", m_autosave_enable->GetValue());
    config->Write("/Autosave/Interval", m_autosave_mins->GetValue());
    config->Write("/UI/NodeRadius", m_node_size->GetValue());
    config->Write("/UI/NodeScheme", m_scheme->GetStringSelection());
    config->Write("/UI/GridSnap", m_grid_snapping->GetStringSelection());
    config->Write("/Sim/MaxSSIterations", m_ss_iterations_max->GetValue());
    config->Write("/Sim/SSRelaxation", m_ss_relaxation->GetValue());
    config->Write("/Sim/MaxSSTolerance", m_ss_tolerance_max->GetValue());
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