/////////////////////////////////////////////////////////////////////////////
// Name:        wx/docmdi.h
// Purpose:     Frame classes for MDI document/view applications
// Author:      Julian Smart
// Created:     01/02/97
// Copyright:   (c) 1997 Julian Smart
//              (c) 2010 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DOCMDI_H_
#define _WX_DOCMDI_H_

#if wxUSE_MDI_ARCHITECTURE

#include "wx/docview.h"
#include "wx/mdi.h"

import <string>;

// Define MDI versions of the doc-view frame classes. Note that we need to
// define them as classes for wxRTTI, otherwise we could simply define them as
// typedefs.

// ----------------------------------------------------------------------------
// An MDI document parent frame
// ----------------------------------------------------------------------------

using wxDocMDIParentFrameBase = wxDocParentFrameAny<wxMDIParentFrame>;

class wxDocMDIParentFrame : public wxDocMDIParentFrameBase
{
public:
    wxDocMDIParentFrame() = default;

    wxDocMDIParentFrame(wxDocManager *manager,
                        wxFrame *parent,
                        wxWindowID id,
                        const std::string& title,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxDEFAULT_FRAME_STYLE,
                        std::string_view name = wxFrameNameStr)
        : wxDocMDIParentFrameBase(manager,
                                  parent, id, title, pos, size, style, name)
    {
    }

    wxDocMDIParentFrame& operator=(wxDocMDIParentFrame&&) = delete;

private:
    wxDECLARE_CLASS(wxDocMDIParentFrame);
};

// ----------------------------------------------------------------------------
// An MDI document child frame
// ----------------------------------------------------------------------------

using wxDocMDIChildFrameBase = wxDocChildFrameAny<wxMDIChildFrame, wxMDIParentFrame>;

class wxDocMDIChildFrame : public wxDocMDIChildFrameBase
{
public:
    wxDocMDIChildFrame() = default;

    wxDocMDIChildFrame(wxDocument *doc,
                       wxView *view,
                       wxMDIParentFrame *parent,
                       wxWindowID id,
                       const std::string& title,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       unsigned int style = wxDEFAULT_FRAME_STYLE,
                       std::string_view name = wxFrameNameStr)
        : wxDocMDIChildFrameBase(doc, view,
                                 parent, id, title, pos, size, style, name)
    {
    }

    wxDocMDIChildFrame& operator=(wxDocMDIChildFrame&&) = delete;

private:
    wxDECLARE_CLASS(wxDocMDIChildFrame);
};

#endif // wxUSE_MDI_ARCHITECTURE

#endif // _WX_DOCMDI_H_
