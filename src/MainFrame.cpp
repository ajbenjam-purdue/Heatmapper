#include "MainFrame.h"
#include "ThermalCanvas.h"
#include "ThermalSolver.h"
#include <fstream>
#include "json.hpp"

// Map the events
enum {
    ID_RunSteadyState = wxID_HIGHEST + 1,
    ID_RunTransient = wxID_HIGHEST + 2
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_OPEN, MainFrame::OnOpen)
    EVT_MENU(wxID_SAVEAS, MainFrame::OnSaveAs)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(ID_RunSteadyState, MainFrame::OnRunSteadyState)
    EVT_BUTTON(ID_RunSteadyState, MainFrame::OnRunSteadyState)
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
    
    // Build the Canvas
    m_canvas = new ThermalCanvas(this);

    // Properties panel
    wxPanel* properties_panel = new wxPanel(this);
    wxBoxSizer* properties_sizer = new wxBoxSizer(wxVERTICAL); // Sizer for panel
    wxButton* run_ss_button = new wxButton(properties_panel, ID_RunSteadyState, "Solve Steady State");
    wxButton* run_tr_button = new wxButton(properties_panel, ID_RunTransient, "Transient Analysis");
    properties_sizer->Add(run_ss_button, 1, wxEXPAND | wxALL, 8);
    properties_sizer->Add(run_tr_button, 1, wxEXPAND | wxALL, 8);
    properties_panel->SetSizer(properties_sizer);

    // Sizer buildout
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_canvas, 1, wxEXPAND | wxALL, 0);
    sizer->Add(properties_panel, 0, wxEXPAND | wxALL, 0);

    // Set the sizer for the main window
    SetSizer(sizer);

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