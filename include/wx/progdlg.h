/////////////////////////////////////////////////////////////////////////////
// Name:        wx/progdlg.h
// Purpose:     Base header for wxProgressDialog
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROGDLG_H_BASE_
#define _WX_PROGDLG_H_BASE_

#include "wx/defs.h"

#if wxUSE_PROGRESSDLG

/*
 * wxProgressDialog flags
 */
constexpr unsigned int wxPD_CAN_ABORT          = 0x0001;
constexpr unsigned int wxPD_APP_MODAL          = 0x0002;
constexpr unsigned int wxPD_AUTO_HIDE          = 0x0004;
constexpr unsigned int wxPD_ELAPSED_TIME       = 0x0008;
constexpr unsigned int wxPD_ESTIMATED_TIME     = 0x0010;
constexpr unsigned int wxPD_SMOOTH             = 0x0020;
constexpr unsigned int wxPD_REMAINING_TIME     = 0x0040;
constexpr unsigned int wxPD_CAN_SKIP           = 0x0080;


#include "wx/generic/progdlgg.h"

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    // The native implementation requires the use of threads and still has some
    // problems, so it can be explicitly disabled.
    #if wxUSE_THREADS && wxUSE_NATIVE_PROGRESSDLG
        #define wxHAS_NATIVE_PROGRESSDIALOG
        #include "wx/msw/progdlg.h"
    #endif
#endif

// If there is no native one, just use the generic version.
#ifndef wxHAS_NATIVE_PROGRESSDIALOG
    class WXDLLIMPEXP_CORE wxProgressDialog
                           : public wxGenericProgressDialog
    {
    public:
        wxProgressDialog( const wxString& title, const wxString& message,
                          int maximum = 100,
                          wxWindow *parent = NULL,
                          unsigned int style = wxPD_APP_MODAL | wxPD_AUTO_HIDE )
            : wxGenericProgressDialog( title, message, maximum,
                                       parent, style )
        {}

        wxProgressDialog (const  wxProgressDialog &) = delete;
	    wxProgressDialog & operator=(const  wxProgressDialog &) = delete;

        wxClassInfo *wxGetClassInfo() const;
        static wxClassInfo ms_classInfo;
        static wxObject* wxCreateObject();
    };
#endif // !wxHAS_NATIVE_PROGRESSDIALOG

#endif // wxUSE_PROGRESSDLG

#endif // _WX_PROGDLG_H_BASE_
