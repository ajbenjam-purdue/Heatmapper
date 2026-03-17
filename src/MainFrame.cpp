#include "MainFrame.h"

#if defined(__WXMSW__)
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Map the events
enum {
    ID_RunSteadyState = wxID_HIGHEST + 1,
    ID_RunTransient,
    ID_ApplyProperties,
    ID_ToolSelect,
    ID_ToolNode,
    ID_ToolEdge,
    ID_ToolDelete,
    ID_OpenEdgeConfig,
    ID_OpenDiscretizer,
    ID_OpenMaterialLib,
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_OPEN, MainFrame::OnOpen)
    EVT_MENU(wxID_CLEAR, MainFrame::OnClear)
    EVT_MENU(wxID_SAVEAS, MainFrame::OnSaveAs)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(ID_RunSteadyState, MainFrame::OnRunSteadyState)
    EVT_MENU(wxID_PREFERENCES, MainFrame::OnPreferences)
    EVT_BUTTON(ID_RunTransient, MainFrame::OnRunTransient)
    EVT_BUTTON(ID_RunSteadyState, MainFrame::OnRunSteadyState)
    EVT_BUTTON(ID_ApplyProperties, MainFrame::OnApplyProperties)
    EVT_BUTTON(ID_OpenEdgeConfig, MainFrame::OnEdgeConfigButtonClicked)
    EVT_MENU(ID_OpenDiscretizer, MainFrame::OnDiscretizeButtonClicked)
    EVT_MENU(ID_OpenMaterialLib, MainFrame::OnMaterialLibOpened)
    EVT_TOOL(ID_ToolSelect, MainFrame::OnToolSelect)
    EVT_TOOL(ID_ToolNode, MainFrame::OnToolSelect)
    EVT_TOOL(ID_ToolEdge, MainFrame::OnToolSelect)
    EVT_TOOL(ID_ToolDelete, MainFrame::OnToolSelect)
    EVT_CHAR_HOOK(MainFrame::OnCharHook)
wxEND_EVENT_TABLE()

void MainFrame::OnRunSteadyState(wxCommandEvent& event) {

    // Create configuration
    ThermalSolver::SimulationConfig steadyStateConfiguration;
    steadyStateConfiguration.residual_threshold = (double)(wxConfigBase::Get()->ReadDouble("Sim/MaxSSTolerance", 0.1));
    steadyStateConfiguration.max_ss_iterations = (int)(wxConfigBase::Get()->ReadLong("/Sim/MaxSSIterations", 100));
    steadyStateConfiguration.ss_relaxation = (double)(wxConfigBase::Get()->ReadDouble("/Sim/SSRelaxation", 0.75));
    
    std::cout << "Starting with config: " << "MaxSSTolerance=" << steadyStateConfiguration.residual_threshold << ", MaxSSIterations=" << steadyStateConfiguration.max_ss_iterations << ", SSRelaxation=" << steadyStateConfiguration.ss_relaxation << std::endl;

    ThermalSolver::solveSteadyState(m_active_network, steadyStateConfiguration);
    m_canvas->Refresh();
}

void MainFrame::OnRunTransient(wxCommandEvent& event) {

    // Create configuration
    ThermalSolver::SimulationConfig steadyStateConfiguration;
    steadyStateConfiguration.residual_threshold = (double)(wxConfigBase::Get()->ReadDouble("Sim/MaxSSTolerance", 0.1));
    steadyStateConfiguration.max_ss_iterations = (int)(wxConfigBase::Get()->ReadLong("/Sim/MaxSSIterations", 100));
    steadyStateConfiguration.ss_relaxation = (double)(wxConfigBase::Get()->ReadDouble("/Sim/SSRelaxation", 0.75));
    steadyStateConfiguration.delta_t = (double)(wxConfigBase::Get()->ReadDouble("/Sim/DefaultDt", 0.01));

    TransientDialog dlg(this, steadyStateConfiguration.delta_t);

    if (dlg.ShowModal() == wxID_OK)
    {
        steadyStateConfiguration.stop_on_steady_state = dlg.GetSteadyStateEnd();
        steadyStateConfiguration.delta_t = dlg.GetTimeStep();
        steadyStateConfiguration.max_time = dlg.GetEndCriteria();

        std::string csv_save_filepath = dlg.GetSaveFilePath();
        std::cout << "Savepath: " << csv_save_filepath << "\n" << "Starting with config: " << "MaxSSTolerance=" << steadyStateConfiguration.residual_threshold << ", MaxSSIterations=" << steadyStateConfiguration.max_ss_iterations << ", SSRelaxation=" << steadyStateConfiguration.ss_relaxation << std::endl;

        ThermalSolver::solveTransient(m_active_network, steadyStateConfiguration, csv_save_filepath);
    }
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

// The constructor implementation
MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size), 
          m_active_network("Workspace Network") { // Initialize the empty network
    
    // Windows application icon
    #if defined(__WXMSW__)
        SetIcon(wxICON(AppIcon));
    #endif

    // Preferences
    m_prefs_editor.AddPage(new GeneralPrefsPage());

    // Build the Canvas
    m_canvas = new ThermalCanvas(this);

    // Materials lib build-out
    wxString userDataDir = wxStandardPaths::Get().GetUserDataDir();
    wxString sep = wxFileName::GetPathSeparator();
    
    // Ensure folder exists before trying a write
    std::filesystem::path dir(userDataDir.ToStdString());
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    // Construct the absolute path to materials.json
    matFilePath = userDataDir + sep + "materials.json";

    // Load it
    m_materials.load_json(matFilePath.ToStdString());

    // Properties panel
    wxPanel* properties_panel = new wxPanel(this);
    properties_panel->SetMinSize(wxSize(250, -1)); // resize
    wxBoxSizer* properties_sizer = new wxBoxSizer(wxVERTICAL); // Sizer for panel
    wxButton* run_ss_button = new wxButton(properties_panel, ID_RunSteadyState, "Solve Steady State");
    wxButton* run_tr_button = new wxButton(properties_panel, ID_RunTransient, "Transient Analysis");
    m_node_label = new wxStaticText(properties_panel, wxID_ANY, "Select a node...");
    properties_sizer->Add(m_node_label, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    m_node_label_str = new wxTextCtrl(properties_panel, wxID_ANY, "");
    properties_sizer->Add(m_node_label_str, 0, wxALL | wxEXPAND, 5);
    m_node_label_str->Hide();

    m_temp_label = new wxStaticText(properties_panel, wxID_ANY, "Temperature (°C):");
    properties_sizer->Add(m_temp_label, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    m_temp_input = new wxTextCtrl(properties_panel, wxID_ANY, "");
    properties_sizer->Add(m_temp_input, 0, wxALL | wxEXPAND, 5);

    m_is_fixed_checkbox = new wxCheckBox(properties_panel, wxID_ANY, "Fix Temperature");
    properties_sizer->Add(m_is_fixed_checkbox, 0, wxALL | wxEXPAND, 5);

    m_load_label = new wxStaticText(properties_panel, wxID_ANY, "Heat Load (W):");
    properties_sizer->Add(m_load_label, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    m_load_input = new wxTextCtrl(properties_panel, wxID_ANY, "");
    properties_sizer->Add(m_load_input, 0, wxALL | wxEXPAND, 5);

    m_thermal_res_label = new wxStaticText(properties_panel, wxID_ANY, "Thermal Resistance (W/K):");
    properties_sizer->Add(m_thermal_res_label, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    m_res_input = new wxTextCtrl(properties_panel, wxID_ANY, "");
    properties_sizer->Add(m_res_input, 0, wxALL | wxEXPAND, 5);

    m_flow_disp_label = new wxStaticText(properties_panel, wxID_ANY, "Calculated Heat Flow (W):");
    properties_sizer->Add(m_flow_disp_label, 0, wxLEFT | wxRIGHT | wxTOP, 5);
    m_flow_disp = new wxTextCtrl(properties_panel, wxID_ANY, "");
    m_flow_disp->Disable(); // Make nonfunctional (view only)
    properties_sizer->Add(m_flow_disp, 0, wxALL | wxEXPAND, 5);
    m_edge_config_button = new wxButton(properties_panel, ID_OpenEdgeConfig, "Open Edge Configuration Tool");
    properties_sizer->Add(m_edge_config_button, 0, wxALL | wxEXPAND, 8);

    // Give this button a new custom ID like ID_ApplyProperties
    m_apply_button = new wxButton(properties_panel, ID_ApplyProperties, "Apply Changes");
    properties_sizer->Add(m_apply_button, 0, wxALL | wxEXPAND, 5);
    properties_sizer->AddStretchSpacer(1);
    properties_sizer->Add(run_ss_button, 0, wxEXPAND | wxALL, 8);
    properties_sizer->Add(run_tr_button, 0, wxEXPAND | wxALL, 8);
    properties_panel->SetSizer(properties_sizer);

    // Sizer buildout
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_canvas, 1, wxEXPAND | wxALL, 0);
    sizer->Add(properties_panel, 0, wxEXPAND | wxALL, 0);

    // Set the sizer for the main window
    SetSizer(sizer);

    /// Reset so it doesn't show random data
    ResetPropertiesWindow();

    // Populate the network with some test data 
    double default_temperature;
    default_temperature = get_default_temperature();
    m_active_network.add_node(ThermalNode(0.25, 0.5, 1.0, 500.0, "Node A", 0, default_temperature));
    m_active_network.add_node(ThermalNode(0.75, 0.5, 1.0, 500.0, "Node B", 1, default_temperature));
    
    // Connect them
    m_active_network.add_edge(ThermalEdge(0, 1, PureResistance{10.0}));

    // Hand the network to the canvas
    m_canvas->SetNetwork(&m_active_network);

    // Get asset paths
    wxString resDir = wxStandardPaths::Get().GetResourcesDir();

    // Load SVGs
    wxToolBar* toolBar = CreateToolBar(wxTB_FLAT | wxTB_NODIVIDER | wxBORDER_NONE);
    toolBar->AddTool(ID_ToolSelect, "Select", wxNullBitmap); // Temporary bitmap
    toolBar->AddTool(ID_ToolNode, "Add Node", wxNullBitmap);
    toolBar->AddTool(ID_ToolEdge, "Add Edge", wxNullBitmap);
    toolBar->AddTool(ID_ToolDelete, "Delete", wxNullBitmap);
    toolBar->Realize(); 
    
    UpdateToolbarIcons(); // Paint the correct SVGs
    UpdateDynamicMenus(); // Build the correct menu
}

void MainFrame::OnClear(wxCommandEvent& event) {
    if (std::max(m_active_network.network_nodes.size(), m_active_network.network_edges.size()) >= 4) // Either N or E >= 4
    {
        // Warn if very populated
        int answer = wxMessageBox(std::format("Do you want to proceed? You have {} node(s) and {} edge(s) in the workspace.", m_active_network.network_nodes.size(), m_active_network.network_edges.size()), "Confirm workspace clearing", 
            wxYES_NO | wxICON_WARNING, this);
        
        if (answer != wxYES)
        {
            return;
        }
    }

    // Clear network
    m_active_network.network_clear();
    m_canvas->m_sel_node_ids.clear();
    m_canvas->m_sel_edge_index = -1;
    
    // Clear the UI
    ResetPropertiesWindow();
    
    Refresh();
}

void MainFrame::OnSaveAs(wxCommandEvent& event) {
    // Open the native save dialog
    wxFileDialog saveFileDialog(this, "Save Thermal Network", "", "",
                                "JSON files (*.json)|*.json", 
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                                
    // If the user clicks "Cancel" abort
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;     
    }

    // Get JSON object from the network
    nlohmann::json j = m_active_network.to_json();

    // Write it to the path the user selected
    std::ofstream file(saveFileDialog.GetPath().ToStdString());
    if (file.is_open()) {
        file << j.dump(4); // 4 space indentation
        file.close();
    } else {
        wxLogError("Cannot save current contents in file '%s'.", saveFileDialog.GetPath());
    }
}

void MainFrame::OnOpen(wxCommandEvent& event) {
    // Open the native open dialog
    wxFileDialog openFileDialog(this, "Open Thermal Network", "", "",
                                "JSON files (*.json)|*.json", 
                                wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    // If the user clicks "Cancel", just abort
    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;     
    }

    // Read the file into a JSON object
    std::ifstream file(openFileDialog.GetPath().ToStdString());
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;
        file.close();

        // Rebuild the network and give it to the canvas
        m_active_network = ThermalNetwork::from_json(j);
        m_canvas->SetNetwork(&m_active_network);
        
    } else {
        wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
    }
}

void MainFrame::ResetPropertiesWindow() {
    m_currently_editing_node = -1;
    m_currently_editing_edge = -1;

    // Hide everything
    m_node_label->Hide();
    m_node_label_str->Hide();
    m_temp_input->Hide();
    m_temp_label->Hide();
    m_is_fixed_checkbox->Hide();
    m_load_input->Hide();
    m_load_label->Hide();
    m_res_input->Hide();
    m_thermal_res_label->Hide();
    m_flow_disp->Hide();
    m_flow_disp_label->Hide();
    m_edge_config_button->Hide();
    m_apply_button->Hide();

    this->Layout(); // Update layout
    UpdateDynamicMenus(); // Update menus
}

void MainFrame::ShowNodeProperties(int node_id) {
    m_currently_editing_node = node_id;
    m_currently_editing_edge = -1;

    // If no node is selected, reset everything and just show the prompt
    if (node_id == -1) {
        ResetPropertiesWindow();
        m_node_label->SetLabel("Select a node...");
        m_node_label->Show();
        m_node_label_str->Hide();
        this->Layout();
        return;
    }

    // Handle Multiple Selection State (-2)
    if (node_id == -2) {
        ResetPropertiesWindow();
        m_node_label->SetLabel("Multiple Nodes Selected");
        m_node_label->Show();
        m_node_label_str->Hide();
        this->Layout();
        return;
    }

    // Otherwise, show the node properties!
    m_node_label->Show();
    m_node_label_str->Show();
    m_temp_input->Show();
    m_temp_label->Show();
    m_is_fixed_checkbox->Show();
    m_load_input->Show();
    m_load_label->Show();
    m_apply_button->Show();
    
    // Ensure edge properties stay hidden
    m_res_input->Hide();
    m_thermal_res_label->Hide();
    m_flow_disp->Hide();
    m_flow_disp_label->Hide();
    m_edge_config_button->Hide();

    // Check for real nodes present
    if (m_active_network.network_nodes.count(node_id) == 0) {
        ResetPropertiesWindow(); // Safety fallback if node doesn't exist
        return; 
    }

    // Grab the node and populate the text boxes
    ThermalNode& node = m_active_network.network_nodes[node_id];
    
    m_node_label->SetLabel(wxString::Format("Editing Node (ID: %d)", node_id));
    m_node_label_str->SetValue(wxString::Format("%s", node.property_label));
    m_temp_input->SetValue(wxString::Format("%.2f", node.node_temperature));
    m_load_input->SetValue(wxString::Format("%.2f", node.ext_load));
    m_is_fixed_checkbox->SetValue(node.is_fixed_temperature);

    this->Layout();
    UpdateDynamicMenus();
}


void MainFrame::ShowEdgeProperties(int edge_index) {
    m_currently_editing_edge = edge_index;
    m_currently_editing_node = -1;

    // If no edge is selected, just clear the window
    if (edge_index == -1) {
        ResetPropertiesWindow();
        return;
    }

    // Show edge properties
    m_res_input->Show();
    m_thermal_res_label->Show();
    m_flow_disp->Show();
    m_flow_disp_label->Show();
    m_edge_config_button->Show();
    m_apply_button->Show();

    // Ensure node properties stay hidden
    m_node_label->Hide();
    m_node_label_str->Hide();
    m_temp_label->Hide();
    m_temp_input->Hide();
    m_is_fixed_checkbox->Hide();
    m_load_input->Hide();
    m_load_label->Hide();

    // Grab the edge and populate the text boxes
    ThermalEdge& edge = m_active_network.network_edges[edge_index];
    double t1 = m_active_network.network_nodes[edge.id_0].node_temperature;
    double t2 = m_active_network.network_nodes[edge.id_1].node_temperature;
    
    m_res_input->SetValue(wxString::Format("%.4f", edge.resistance(t1, t2)));
    m_flow_disp->SetValue(wxString::Format("%.2f", std::abs(m_active_network.get_edge_flux(edge_index))));

    this->Layout();
    UpdateDynamicMenus();
}

void MainFrame::OnApplyProperties(wxCommandEvent& event) {
    if (m_currently_editing_node != -1) 
    {

        ThermalNode& node = m_active_network.network_nodes[m_currently_editing_node];

        // Read the text from the UI
        wxString temp_str = m_temp_input->GetValue();
        wxString load_str = m_load_input->GetValue();
        wxString label_str = m_node_label_str->GetValue();

        // Apply node label
        node.property_label = label_str;

        double new_temp, new_load;

        // ToDouble() safely checks if the user typed valid numbers!
        if (temp_str.ToDouble(&new_temp)) {
            node.node_temperature = new_temp;
            
            // If they explicitly change the temp, fix it as a boundary condition
            node.fixTemperature(new_temp); 
        }

        if (load_str.ToDouble(&new_load)) {
            node.applyHeatLoad(new_load);
        }

        if (m_is_fixed_checkbox->GetValue()) {
            // If box is checked, make sure to apply BC
            node.fixTemperature(new_temp); 
        } else {
            // Unlock it and apply the heat load instead
            node.is_fixed_temperature = false;
            node.node_temperature = new_temp; 
            node.applyHeatLoad(new_load);     
        }
    }
    else if (m_currently_editing_edge != -1)
    {
        ThermalEdge& edge = m_active_network.network_edges[m_currently_editing_edge];
        
        double new_res;
        if (m_res_input->GetValue().ToDouble(&new_res)) {
            // Assign directly -> stand in
            edge.params = PureResistance{std::max(new_res, 1e-8)}; 
        }
    }
    // Tell the canvas to redraw with the new numbers
    m_canvas->Refresh();
}

void MainFrame::OnToolSelect(wxCommandEvent& event) {
    if (event.GetId() == ID_ToolSelect) m_canvas->SetToolMode(ToolMode::SELECT);
    else if (event.GetId() == ID_ToolNode) m_canvas->SetToolMode(ToolMode::ADD_NODE);
    else if (event.GetId() == ID_ToolEdge) m_canvas->SetToolMode(ToolMode::ADD_EDGE);
    else if (event.GetId() == ID_ToolDelete) m_canvas->SetToolMode(ToolMode::DELETE_ITEM);

    UpdateToolbarIcons();
}

void MainFrame::ForceSelectTool() {
    m_canvas->SetToolMode(ToolMode::SELECT);
    UpdateToolbarIcons();
}

// Fake toolbar
void MainFrame::UpdateToolbarIcons() {
    wxString resDir = wxStandardPaths::Get().GetResourcesDir();
    wxString sep = wxFileName::GetPathSeparator();
    wxToolBar* tb = GetToolBar();
    
    // Reset everything to the default, inactive SVGs
    tb->SetToolNormalBitmap(ID_ToolSelect, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "select.svg", wxSize(24, 24)));
    tb->SetToolNormalBitmap(ID_ToolNode, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "add_node.svg", wxSize(24, 24)));
    tb->SetToolNormalBitmap(ID_ToolEdge, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "add_edge.svg", wxSize(24, 24)));
    tb->SetToolNormalBitmap(ID_ToolDelete, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "delete.svg", wxSize(24, 24)));

    // Overwrite the currently active tool with its "_selected" version
    ToolMode active = m_canvas->m_current_tool;
    if (active == ToolMode::SELECT) tb->SetToolNormalBitmap(ID_ToolSelect, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "select_selected.svg", wxSize(24, 24)));
    else if (active == ToolMode::ADD_NODE) tb->SetToolNormalBitmap(ID_ToolNode, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "add_node_selected.svg", wxSize(24, 24)));
    else if (active == ToolMode::ADD_EDGE) tb->SetToolNormalBitmap(ID_ToolEdge, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "add_edge_selected.svg", wxSize(24, 24)));
    else if (active == ToolMode::DELETE_ITEM) tb->SetToolNormalBitmap(ID_ToolDelete, wxBitmapBundle::FromSVGFile(resDir + sep + "assets" + sep + "delete_selected.svg", wxSize(24, 24)));
}

bool IsTextControlFocused() {
    wxWindow* focused = wxWindow::FindFocus();
    return focused && wxDynamicCast(focused, wxTextCtrl) != nullptr;
}

void MainFrame::OnCharHook(wxKeyEvent& event) {
    // If a text box is focused, let it handle ALL its own typing normally!
    wxWindow* focused = wxWindow::FindFocus();
    if (focused && wxDynamicCast(focused, wxTextCtrl) != nullptr) {
        event.Skip(); // Passes the raw keystroke down to the text box
        return;
    }

    // Otherwise, process hotkeys!
    int key = event.GetKeyCode();
    bool cmdDown = event.CmdDown();

    if (key == WXK_DELETE || key == WXK_BACK) {
        m_canvas->DeleteSelectedItems();
    }
    else if (cmdDown && key == 'C') {
        m_canvas->CopySelected();
    }
    else if (cmdDown && key == 'V') {
        m_canvas->PasteSelected();
    }
    else {
        // Not a hotkey we care about, let the system handle it normally
        event.Skip(); 
    }
}

void MainFrame::UpdateDynamicMenus() {
    // Create a brand new Menu Bar
    wxMenuBar* new_bar = new wxMenuBar;

    // Build the permanent menus (File & Run)
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN, "&Load from .json\tCtrl-O", "Open a thermal network JSON file");
    menuFile->Append(wxID_SAVEAS, "Save &As .json\tCtrl-Shift-S", "Save the thermal network to JSON");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_CLEAR, "&Reset workspace\tCtrl-Shift-C", "Reset the current workspace");
    menuFile->Append(ID_OpenMaterialLib, "Open Materials Library\tCtrl-Shift-M", "Open the Materials Library to Add, Edit, or Remove entries");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_PREFERENCES);
    menuFile->Append(wxID_EXIT);

    wxMenu* menuRun = new wxMenu;
    menuRun->Append(ID_RunSteadyState, "&Run Static Analysis\tCtrl-R", "Run the steady-state configuration");
    menuRun->Append(ID_RunTransient, "&Run Transient Analysis\tCtrl-Alt-R", "Perform transient analysis");

    new_bar->Append(menuFile, "&File");
    new_bar->Append(menuRun, "&Run");

    // Conditionally build and attach the Node/Edge menu
    bool has_nodes = (m_currently_editing_node != -1) || (m_currently_editing_node == -2);
    bool has_edges = (m_currently_editing_edge != -1);
    if (has_nodes) {
        wxMenu* nodeMenu = new wxMenu;
        nodeMenu->Append(ID_OpenDiscretizer, "Discretize Node\tCtrl-D", "Replace single node with a multi-node representation");
        new_bar->Append(nodeMenu, "&Node");
    }
    else if (has_edges) {
        wxMenu* edgeMenu = new wxMenu;
        edgeMenu->Append(wxID_ANY, "Set Perfect Conductor", "Drop thermal resistance to zero");
        new_bar->Append(edgeMenu, "&Edge");
    }

    // Swap the entire bar to avoid destructor inference
    SetMenuBar(new_bar);
}

void MainFrame::OnEdgeConfigButtonClicked(wxCommandEvent& event)
{
    if (m_currently_editing_edge == -1) return;

    EdgeConfigDialog dialog(this, m_materials);
    
    // ShowModal() pauses the app
    if (dialog.ShowModal() == wxID_OK)
    { // Update if OK was hit
        double new_r = dialog.GetCalculatedResistance(); // NOT USED

        // Assign edge params to edge
        m_active_network.network_edges[m_currently_editing_edge].params = dialog.GetEdgeParameters(); 
        
        // To update the UI Preview box edge needs to calculate its own value
        ThermalEdge& edge = m_active_network.network_edges[m_currently_editing_edge];
        double t1 = m_active_network.network_nodes[edge.id_0].node_temperature;
        double t2 = m_active_network.network_nodes[edge.id_1].node_temperature;

        m_res_input->SetValue(wxString::Format("%.4f", edge.resistance(t1, t2)));
        
        m_canvas->Refresh();
    }
}

void MainFrame::OnDiscretizeButtonClicked(wxCommandEvent& event) {
    if (m_currently_editing_node < 0) return;

    DiscretizeDialog dialog(this, m_materials);
    
    if (dialog.ShowModal() == wxID_OK) {

        // Extract information if the user clicked ok and not cancel/esc
        int type = dialog.GetDiscretizationType();
        int N = std::max(1, dialog.GetN());
        int M = std::max(1, dialog.GetM());
        
        // Grab the advanced resistances!
        double R1 = std::max(dialog.GetR1(), 1e-8);
        double R2 = std::max(dialog.GetR2(), 1e-8);

        // Capture the original node properties
        ThermalNode old_node = m_active_network.network_nodes[m_currently_editing_node];
        int old_id = old_node.node_id;
        
        // Calculate the physical fractions
        int total_new_nodes = (type == 2) ? (N * M) : ((type == 1) ? (N + 1) : N);
        double sub_mass = old_node.property_mass / total_new_nodes;
        double sub_load = old_node.ext_load / total_new_nodes;

        // Clear the old node from memory (but keep the edges untouched for a moment)
        m_active_network.network_nodes.erase(old_id);
        m_canvas->m_sel_node_ids.clear();
        ResetPropertiesWindow();

        std::vector<int> new_ids;

        // Generate the Sub-Network
        if (type == 0) { 
            // N-Node Chain
            for (int i = 0; i < N; ++i) {
                double nx = old_node.canvas_position_x + (i - N/2.0) * 0.08; 
                ThermalNode nn(nx, old_node.canvas_position_y, sub_mass, old_node.property_specific_heat, 
                               old_node.property_label + "_ch" + std::to_string(i), 0, old_node.node_temperature);
                nn.ext_load = sub_load;
                if (old_node.is_fixed_temperature) nn.fixTemperature(old_node.node_temperature);
                new_ids.push_back(m_active_network.add_node(nn));
            }
            // Wire together
            for (int i = 0; i < N - 1; ++i) {
                m_active_network.add_edge(ThermalEdge(new_ids[i], new_ids[i+1], PureResistance{R1})); // Use R1
            }
        }
        else if (type == 1) { 
            // N-Node Centralized Hub (Center node + N surrounding spokes)
            ThermalNode center(old_node.canvas_position_x, old_node.canvas_position_y, sub_mass, old_node.property_specific_heat, 
                               old_node.property_label + "_hub", 0, old_node.node_temperature);
            center.ext_load = sub_load;
            if (old_node.is_fixed_temperature) center.fixTemperature(old_node.node_temperature);
            int center_id = m_active_network.add_node(center);
            new_ids.push_back(center_id);

            for (int i = 0; i < N; ++i) {
                double angle = (2.0 * M_PI * i) / N;
                double nx = old_node.canvas_position_x + std::cos(angle) * 0.08;
                double ny = old_node.canvas_position_y + std::sin(angle) * 0.08;
                
                ThermalNode spoke(nx, ny, sub_mass, old_node.property_specific_heat, 
                                  old_node.property_label + "_sp" + std::to_string(i), 0, old_node.node_temperature);
                spoke.ext_load = sub_load;
                if (old_node.is_fixed_temperature) spoke.fixTemperature(old_node.node_temperature);
                
                int spoke_id = m_active_network.add_node(spoke);
                new_ids.push_back(spoke_id);
                m_active_network.add_edge(ThermalEdge(center_id, spoke_id, PureResistance{R1})); // Use R1
            }
        }
        else if (type == 2) { 
            // NxM Comb (N teeth, M nodes deep)
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < M; ++j) { // Default sep 0.08. Will parametrize later
                    double nx = old_node.canvas_position_x + (i - N/2.0) * 0.08; 
                    double ny = old_node.canvas_position_y + (j * 0.08);
                    ThermalNode nn(nx, ny, sub_mass, old_node.property_specific_heat, 
                                   old_node.property_label + "_c" + std::to_string(i) + "-" + std::to_string(j), 0, old_node.node_temperature);
                    nn.ext_load = sub_load;
                    if (old_node.is_fixed_temperature) nn.fixTemperature(old_node.node_temperature);
                    
                    int id = m_active_network.add_node(nn);
                    new_ids.push_back(id);

                    // Wire vertical teeth (Use R2!)
                    if (j > 0) m_active_network.add_edge(ThermalEdge(new_ids.back() - 1, new_ids.back(), PureResistance{R2}));
                    
                    // Wire the horizontal spine (Use R1!)
                    if (j == 0 && i > 0) m_active_network.add_edge(ThermalEdge(new_ids[new_ids.size() - 1 - M], new_ids.back(), PureResistance{R1}));
                }
            }
        }

        // Re-wire the dangling edges
        // Root of the new mesh is new_ids[0] -> basis for all other nodes
        int root_id = new_ids[0];
        
        for (ThermalEdge& edge : m_active_network.network_edges) {
            if (edge.id_0 == old_id) edge.id_0 = root_id;
            if (edge.id_1 == old_id) edge.id_1 = root_id;
        }

        m_canvas->Refresh();
    }
}

void MainFrame::OnMaterialLibOpened(wxCommandEvent& event)
{
    MaterialDialog dialog(this, m_materials);
    if (dialog.ShowModal() == wxID_OK) {
        // Get new library and set to current
        m_materials = dialog.GetModifiedLibrary();
        
        // Save to the existing path already used to load
        m_materials.save_json(matFilePath.ToStdString()); 
    }
}

void MainFrame::OnPreferences(wxCommandEvent& event)
{
    m_prefs_editor.Show(this);
}