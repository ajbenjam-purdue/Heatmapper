#pragma once
#include <wx/wx.h>
#include <wx/snglinst.h>
#include <wx/cmdline.h>

class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;

    // Improved exception handling
    virtual bool OnExceptionInMainLoop() override;
    virtual void OnUnhandledException() override;

    // Cmd line parsing for file i/o
    virtual void OnInitCmdLine(wxCmdLineParser& parser) override;
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser) override;

    #ifdef __WXMAC__
        virtual void MacOpenFiles(const wxArrayString& fileNames) override;
    #endif

private:
    wxSingleInstanceChecker* m_checker = nullptr;
    wxString m_startup_file = ""; // Holds the file path if launched via double-click
};

DECLARE_APP(MyApp) // Instance checking