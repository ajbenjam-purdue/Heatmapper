#include "App.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MainFrame *frame = new MainFrame("Heatmapper", wxPoint(30, 50), wxSize(900, 500));
    frame->Show(true);
    return true;
}