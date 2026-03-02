#include "App.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MainFrame *frame = new MainFrame("Heatmapper", wxPoint(50, 50), wxSize(450, 340));
    frame->Show(true);
    return true;
}