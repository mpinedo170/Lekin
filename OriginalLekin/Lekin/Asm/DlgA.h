#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDlgA dialog
// Implements common functions of some dialogs in the program

class CDlgA : public CDlg
{
private:
    typedef CDlg super;

public:
    CDlgA();
    explicit CDlgA(UINT nIDTemplate);

    void ModifyLab();
    void FillWorkspaces();
    void FillObjectives();
    void SetStdLimitText();

protected:
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};
