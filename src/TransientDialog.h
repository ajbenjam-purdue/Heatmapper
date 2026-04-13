#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <string>

class TransientDialog : public wxDialog
{
public:
    TransientDialog(wxWindow* parent, double dt, bool save_csv, bool save_compressed);

    bool GetSteadyStateEnd() const;
    double GetEndCriteria() const;
    double GetTimeStep() const;
    std::string GetSaveFilePath() const;
    bool GetSaveCSV() const;
    bool GetSaveCompressedCSV() const;

private:
    wxCheckBox* m_checkbox;
    wxCheckBox* m_save_csv_checkbox;
    wxCheckBox* m_save_compressed_checkbox;
    wxSpinCtrlDouble* m_end_criteria_input;
    wxSpinCtrlDouble* m_dt_input;
    wxTextCtrl* m_csv_path;
    wxButton* m_browse_button;

    void OnCheck(wxCommandEvent& event);
    void OnBrowseCSV(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};