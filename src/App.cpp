#include "App.h"
#include "MainFrame.h"
#include <exception>

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    SetVendorName("ajbenjam");
    SetAppName("Heatmapper");

    // Mutex check
    m_checker = new wxSingleInstanceChecker("Heatmapper_Instance_Lock_" + wxGetUserId());
    if (m_checker->IsAnotherRunning()) {
        wxLogError("Another instance of Heatmapper is already running.");
        delete m_checker;
        return false; // Abort
    }

    // Call default init which triggers command line parsing
    if (!wxApp::OnInit()) return false; 

    // Boot the UI
    MainFrame* frame = new MainFrame("Heatmapper", wxDefaultPosition, wxSize(1200, 800));
    frame->Show(true);

    if (!m_startup_file.IsEmpty()) {
        // NOTE: You will need to extract your JSON loading logic from MainFrame::OnOpen 
        // into a new public method called `MainFrame::LoadNetworkFromFile(wxString path)`
        frame->LoadNetworkFromFile(m_startup_file); 
    }

    return true;
}

int MyApp::OnExit() {
    delete m_checker;
    return wxApp::OnExit();
}

// --- FEATURE 1: THE SAFETY NET ---
bool MyApp::OnExceptionInMainLoop() {
    try {
        throw; // Re-throw to catch the specific type
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format("A fatal math or memory error occurred:\n\n%s\n\nPlease save your work to a new file and restart the application.", e.what()), 
                     "Critical Error", wxOK | wxICON_ERROR);
    } catch (...) {
        wxMessageBox("An unknown critical error occurred.", "Critical Error", wxOK | wxICON_ERROR);
    }
    return true; // Return true to attempt to keep the app alive!
}

void MyApp::OnUnhandledException() {
    // This is the absolute last resort before macOS kills the app
    wxMessageBox("A catastrophic error occurred. Heatmapper will now close.", "Fatal Crash", wxOK | wxICON_ERROR);
}

// Windows & Linux Terminal Parsing
void MyApp::OnInitCmdLine(wxCmdLineParser& parser) {
    // 1. Call the base class to load standard wxWidgets flags first
    wxApp::OnInitCmdLine(parser);

    // 2. Append our custom optional file parameter to the end
    parser.AddParam("File to open", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
}

bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser) {
    if (parser.GetParamCount() > 0) {
        m_startup_file = parser.GetParam(0);
    }
    return wxApp::OnCmdLineParsed(parser);
}

// Double-Click / Drag-in Parsing
#ifdef __WXMAC__
void MyApp::MacOpenFiles(const wxArrayString& fileNames) {
    if (fileNames.GetCount() > 0) {
        // If the app is already open, we just load it directly into the active window
        if (GetTopWindow()) {
            MainFrame* frame = dynamic_cast<MainFrame*>(GetTopWindow());
            if (frame) frame->LoadNetworkFromFile(fileNames[0]);
        } else {
            // App is booting up, save it for OnInit
            m_startup_file = fileNames[0]; 
        }
    }
}
#endif