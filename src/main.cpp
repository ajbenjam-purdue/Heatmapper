#include <wx/wx.h>

// 1. Define the Application Class
class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

// 2. Define the Main Window (Frame) Class
class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE(); // Required for event handling
};

// 3. Map Events to Functions
enum {
    ID_Hello = 1
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_BUTTON(ID_Hello, MyFrame::OnHello)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
wxEND_EVENT_TABLE()

// 4. Implement Application Initialization
wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    // Create the main window instance
    MyFrame *frame = new MyFrame("My First wxWidgets App", wxPoint(50, 50), wxSize(450, 340));
    frame->Show(true);
    return true;
}

// 5. Build the Window and Populate It
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size) {
    
    // Create a panel to hold your widgets (buttons, text, etc.)
    wxPanel *panel = new wxPanel(this, wxID_ANY);

    // Populate the panel with a clickable button
    wxButton *button = new wxButton(panel, ID_Hello, "Click Me!", wxPoint(150, 100), wxSize(150, 40));
    
    // Add a status bar to the bottom of the window
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
}

// 6. Define What Happens on Interaction
void MyFrame::OnHello(wxCommandEvent& event) {
    wxMessageBox("Hello from wxWidgets on Apple Silicon!", "Greetings", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}