#include "App.h"
#include "MainFrame.h"
#include <wx/config.h>

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {

    // Set application name for appData/program files dir
    SetAppName("Heatmapper");
    wxConfigBase::Set(new wxConfig("Heatmapper", "ajbenjam"));

    // Create mainframe instance and show
    MainFrame *frame = new MainFrame("Heatmapper", wxPoint(30, 50), wxSize(900, 500));
    frame->Show(true);
    
    return true;
}