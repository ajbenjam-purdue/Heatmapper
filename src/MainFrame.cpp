#include "MainFrame.h"
#include "ThermalCanvas.h"
#include "ThermalSolver.h"
#include <fstream>
#include <wx/bmpbndl.h>
#include <wx/filename.h>
#include <wx/stdpaths.h> // Icons
#include "json.hpp"

#if defined(__WXMSW__)
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Map the events
enum {
    ID_RunSteadyState = wxID_HIGHEST + 1,
    ID_RunTransient = wxID_HIGHEST + 2,
    ID_ApplyProperties = wxID_HIGHEST + 3,
    ID_ToolSelect = wxID_HIGHEST + 4,
    ID_ToolNode = wxID_HIGHEST + 5,
    ID_ToolEdge = wxID_HIGHEST + 6,
    ID_ToolDelete = wxID_HIGHEST + 7
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_OPEN, MainFrame::OnOpen)
    EVT_MENU(wxID_SAVEAS, MainFrame::OnSaveAs)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(ID_RunSteadyState, MainFrame::OnRunSteadyState)
    EVT_BUTTON(ID_RunSteadyState, MainFrame::OnRunSteadyState)
    EVT_BUTTON(ID_ApplyProperties, MainFrame::OnApplyProperties)
    EVT_TOOL(ID_ToolSelect, MainFrame::OnToolSelect)
    EVT_TOOL(ID_ToolNode, MainFrame::OnToolSelect)
    EVT_TOOL(ID_ToolEdge, MainFrame::OnToolSelect)
    EVT_TOOL(ID_ToolDelete, MainFrame::OnToolSelect)
wxEND_EVENT_TABLE()

void MainFrame::OnRunSteadyState(wxCommandEvent& event) {
    ThermalSolver::solveSteadyState(m_active_network);
    m_canvas->Refresh();
}
void MainFrame::OnRunTransient(wxCommandEvent& event) {
    std::cout << "Transient Run" << std::endl;
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

    // Build the Canvas
    m_canvas = new ThermalCanvas(this);

    // Properties panel
    wxPanel* properties_panel = new wxPanel(this);
    wxBoxSizer* properties_sizer = new wxBoxSizer(wxVERTICAL); // Sizer for panel
    wxButton* run_ss_button = new wxButton(properties_panel, ID_RunSteadyState, "Solve Steady State");
    wxButton* run_tr_button = new wxButton(properties_panel, ID_RunTransient, "Transient Analysis");
    m_node_label = new wxStaticText(properties_panel, wxID_ANY, "Select a node...");
    properties_sizer->Add(m_node_label, 0, wxALL | wxEXPAND, 5);

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
    m_flow_disp->Disable();
    properties_sizer->Add(m_flow_disp, 0, wxALL | wxEXPAND, 5);

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
    m_active_network.add_node(ThermalNode(0.25, 0.5, 1.0, 500.0, "Node A", 0));
    m_active_network.add_node(ThermalNode(0.75, 0.5, 1.0, 500.0, "Node B", 1));
    
    // Connect them
    m_active_network.add_edge(ThermalEdge(0, 1, PureResistance{10.0}));

    // Hand the network to the canvas
    m_canvas->SetNetwork(&m_active_network);

    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_OPEN, "&Load from .json\tCtrl-O", "Open a thermal network JSON file");
    menuFile->Append(wxID_SAVEAS, "Save &As .json\tCtrl-Shift-S", "Save the thermal network to JSON");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu *menuRun = new wxMenu;
    menuRun->Append(ID_RunSteadyState, "&Run Static Analysis\tCtrl-R", "Run the steady-state configuration");
    menuRun->Append(ID_RunTransient, "&Run Transient Analysis\tCtrl-Alt-R", "Perform transient analysis on the current configuration");

    // Add it to the Menu Bar and attach it to the Frame
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuRun, "&Run");
    SetMenuBar(menuBar);

    // Toolbar

    // Get asset paths
    wxString resDir = wxStandardPaths::Get().GetResourcesDir();
    wxString sep = wxFileName::GetPathSeparator(); 

    // Load SVGs
    wxToolBar* toolBar = CreateToolBar(wxTB_FLAT | wxTB_NODIVIDER | wxBORDER_NONE);
    toolBar->AddTool(ID_ToolSelect, "Select", wxNullBitmap); // Temporary bitmap, we will set it correctly later
    toolBar->AddTool(ID_ToolNode, "Add Node", wxNullBitmap);
    toolBar->AddTool(ID_ToolEdge, "Add Edge", wxNullBitmap);
    toolBar->AddTool(ID_ToolDelete, "Delete", wxNullBitmap);
    toolBar->Realize(); 
    
    UpdateToolbarIcons(); // Paint the correct SVGs
}

void MainFrame::OnSaveAs(wxCommandEvent& event) {
    // 1. Open the native save dialog
    wxFileDialog saveFileDialog(this, "Save Thermal Network", "", "",
                                "JSON files (*.json)|*.json", 
                                wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                                
    // If the user clicks "Cancel", just abort
    if (saveFileDialog.ShowModal() == wxID_CANCEL) {
        return;     
    }

    // 2. Get your JSON object from the network
    nlohmann::json j = m_active_network.to_json();

    // 3. Write it to the path the user selected
    std::ofstream file(saveFileDialog.GetPath().ToStdString());
    if (file.is_open()) {
        file << j.dump(4); // The '4' adds beautiful indentation (4 spaces)
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
    m_temp_input->Hide();
    m_temp_label->Hide();
    m_is_fixed_checkbox->Hide();
    m_load_input->Hide();
    m_load_label->Hide();
    m_res_input->Hide();
    m_thermal_res_label->Hide();
    m_flow_disp->Hide();
    m_flow_disp_label->Hide();
    m_apply_button->Hide();

    this->Layout();
}

void MainFrame::ShowNodeProperties(int node_id) {
    m_currently_editing_node = node_id;
    m_currently_editing_edge = -1;

    // If no node is selected, reset everything and just show the prompt
    if (node_id == -1) {
        ResetPropertiesWindow();
        m_node_label->SetLabel("Select a node...");
        m_node_label->Show();
        this->Layout();
        return;
    }

    // Handle Multiple Selection State (-2)
    if (node_id == -2) {
        ResetPropertiesWindow();
        m_node_label->SetLabel("Multiple Nodes Selected");
        m_node_label->Show();
        this->Layout();
        return;
    }

    // Otherwise, show the node properties!
    m_node_label->Show();
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

    // Grab the node and populate the text boxes
    ThermalNode& node = m_active_network.network_nodes[node_id];
    
    m_node_label->SetLabel(wxString::Format("Editing Node %d", node_id));
    m_temp_input->SetValue(wxString::Format("%.2f", node.node_temperature));
    m_load_input->SetValue(wxString::Format("%.2f", node.ext_load));
    m_is_fixed_checkbox->SetValue(node.is_fixed_temperature);

    this->Layout();
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
    m_apply_button->Show();

    // Ensure node properties stay hidden
    m_node_label->Hide();
    m_temp_label->Hide();
    m_temp_input->Hide();
    m_is_fixed_checkbox->Hide();
    m_load_input->Hide();
    m_load_label->Hide();

    // Grab the edge and populate the text boxes
    ThermalEdge& edge = m_active_network.network_edges[edge_index];
    
    m_res_input->SetValue(wxString::Format("%.2f", edge.resistance()));
    m_flow_disp->SetValue(wxString::Format("%.2f", std::abs(m_active_network.get_edge_flux(edge_index))));

    this->Layout();
}

void MainFrame::OnApplyProperties(wxCommandEvent& event) {
    if (m_currently_editing_node != -1) 
    {

        ThermalNode& node = m_active_network.network_nodes[m_currently_editing_node];

        // Read the text from the UI
        wxString temp_str = m_temp_input->GetValue();
        wxString load_str = m_load_input->GetValue();

        double new_temp, new_load;

        // ToDouble() safely checks if the user typed valid numbers!
        if (temp_str.ToDouble(&new_temp)) {
            node.node_temperature = new_temp;
            
            // If they explicitly change the temp, let's fix it as a boundary condition!
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
            // Assign directly -> stand in. TODO: CHANGE TO COMPLEX DROPDOWN
            edge.params = PureResistance{new_res}; 
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
    
    // 1. Reset everything to the default, inactive SVGs
    tb->SetToolNormalBitmap(ID_ToolSelect, wxBitmapBundle::FromSVGFile(resDir + sep + "select.svg", wxSize(24, 24)));
    tb->SetToolNormalBitmap(ID_ToolNode, wxBitmapBundle::FromSVGFile(resDir + sep + "add_node.svg", wxSize(24, 24)));
    tb->SetToolNormalBitmap(ID_ToolEdge, wxBitmapBundle::FromSVGFile(resDir + sep + "add_edge.svg", wxSize(24, 24)));
    tb->SetToolNormalBitmap(ID_ToolDelete, wxBitmapBundle::FromSVGFile(resDir + sep + "delete.svg", wxSize(24, 24)));

    // 2. Overwrite the currently active tool with its "_selected" glowing version!
    ToolMode active = m_canvas->m_current_tool;
    if (active == ToolMode::SELECT) tb->SetToolNormalBitmap(ID_ToolSelect, wxBitmapBundle::FromSVGFile(resDir + sep + "select_selected.svg", wxSize(24, 24)));
    else if (active == ToolMode::ADD_NODE) tb->SetToolNormalBitmap(ID_ToolNode, wxBitmapBundle::FromSVGFile(resDir + sep + "add_node_selected.svg", wxSize(24, 24)));
    else if (active == ToolMode::ADD_EDGE) tb->SetToolNormalBitmap(ID_ToolEdge, wxBitmapBundle::FromSVGFile(resDir + sep + "add_edge_selected.svg", wxSize(24, 24)));
    else if (active == ToolMode::DELETE_ITEM) tb->SetToolNormalBitmap(ID_ToolDelete, wxBitmapBundle::FromSVGFile(resDir + sep + "delete_selected.svg", wxSize(24, 24)));
}