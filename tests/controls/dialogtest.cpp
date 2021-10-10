///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/dialogtest.cpp
// Purpose:     wxWindow unit test
// Author:      Vaclav Slavik
// Created:     2012-08-30
// Copyright:   (c) 2012 Vaclav Slavik
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"


#include "wx/testing.h"

#ifdef HAVE_VARIADIC_MACROS

#include "wx/msgdlg.h"
#include "wx/filedlg.h"

// This test suite tests helpers from wx/testing.h intended for testing of code
// that calls modal dialogs. It does not test the implementation of wxWidgets'
// dialogs.

TEST_CASE("MessageDialog")
{
    int rc;

#if wxUSE_FILEDLG
    #define FILE_DIALOG_TEST ,\
        wxExpectModal<wxFileDialog>(wxGetCwd() + "/test.txt").Optional()
#else
    #define FILE_DIALOG_TEST
#endif

    wxTEST_DIALOG
    (
        rc = wxMessageBox("Should I fail?", "Question", DialogFlags{wxDialogFlags::Yes, wxDialogFlags::No}),
        wxExpectModal<wxMessageDialog>(wxDialogFlags::No)
        FILE_DIALOG_TEST
    );

    CHECK_EQ(wxDialogFlags::No, rc);
}

#if wxUSE_FILEDLG
TEST_CASE("FileDialog")
{
    wxFileDialog dlg(nullptr);
    int rc;

    wxTEST_DIALOG
    (
        rc = dlg.ShowModal(),
        wxExpectModal<wxFileDialog>(wxGetCwd() + "/test.txt")
    );

    CHECK_EQ((int)wxID_OK, rc);

    CHECK_EQ("test.txt", dlg.GetFilename());

#ifdef __WXGTK3__
    // The native file dialog in GTK+ 3 launches an async operation which tries
    // to dereference the already deleted dialog object if we don't let it to
    // complete before leaving this function.
    wxYield();
#endif
}
#endif

class MyDialog : public wxDialog
{
public:
    MyDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Entry"), m_value(-1)
    {
        // Dummy. Imagine it's a real dialog that shows some number-entry
        // controls.
    }

    int m_value;
};


template<>
class wxExpectModal<MyDialog> : public wxExpectModalBase<MyDialog>
{
public:
    wxExpectModal(int valueToSet) : m_valueToSet(valueToSet) {}

protected:
    int OnInvoked(MyDialog *dlg) const override
    {
        // Simulate the user entering the expected number:
        dlg->m_value = m_valueToSet;
        return wxID_OK;
    }

    int m_valueToSet;
};

TEST_CASE("CustomDialog")
{
    MyDialog dlg(nullptr);

    wxTEST_DIALOG
    (
        dlg.ShowModal(),
        wxExpectModal<MyDialog>(42)
    );

    CHECK_EQ( 42, dlg.m_value );
}


class MyModalDialog : public wxDialog
{
public:
    MyModalDialog() : wxDialog (nullptr, wxID_ANY, "Modal Dialog")
    {
        m_wasModal = false;
        Bind( wxEVT_INIT_DIALOG, &MyModalDialog::OnInit, this );
    }

    void OnInit(wxInitDialogEvent& WXUNUSED(event))
    {
        m_wasModal = IsModal();
        CallAfter( &MyModalDialog::EndModal, wxID_OK );
    }

    bool WasModal() const
    {
        return m_wasModal;
    }

private:
    bool m_wasModal;
};

TEST_CASE("InitDialog")
{
    MyModalDialog dlg;
    dlg.ShowModal();
    CHECK( dlg.WasModal() );
}

#endif // HAVE_VARIADIC_MACROS
