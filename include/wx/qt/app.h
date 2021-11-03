/////////////////////////////////////////////////////////////////////////////
// Name:        app.h
// Purpose:     wxApp class
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2009 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_APP_H_
#define _WX_QT_APP_H_

#include <wx/scopedarray.h>

class QApplication;
class wxApp : public wxAppBase
{
public:
    wxApp();
    ~wxApp();

    wxApp (const  wxApp &) = delete;
    wxApp & operator=(const  wxApp &) = delete;

    bool Initialize(int& argc, wxChar **argv) override;

private:
    std::unique_ptr<QApplication> m_qtApplication;
    int m_qtArgc;
    wxScopedArray<char*> m_qtArgv;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_QT_APP_H_
