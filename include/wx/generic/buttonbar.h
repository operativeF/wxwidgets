///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/buttonbar.h
// Purpose:     wxButtonToolBar declaration
// Author:      Julian Smart, after Robert Roebling, Vadim Zeitlin, SciTech
// Modified by:
// Created:     2006-04-13
// Copyright:   (c) Julian Smart, Robert Roebling, Vadim Zeitlin,
//              SciTech Software, Inc.
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BUTTONBAR_H_
#define _WX_BUTTONBAR_H_

#include "wx/bmpbuttn.h"
#include "wx/toolbar.h"

import Utils.Geometry;

import <string>;

class wxButtonToolBarTool;


// ----------------------------------------------------------------------------
// wxButtonToolBar
// ----------------------------------------------------------------------------

class wxButtonToolBar : public wxToolBarBase
{
public:
    // construction/destruction
    wxButtonToolBar() { Init(); }
    wxButtonToolBar(wxWindow *parent,
              wxWindowID id,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = 0,
              std::string_view name = wxToolBarNameStr)
    {
        Init();

        Create(parent, id, pos, size, style, name);
    }

    bool Create( wxWindow *parent,
                 wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = 0,
                 std::string_view name = wxToolBarNameStr);

    bool Realize() override;

    void SetToolShortHelp(int id, const std::string& helpString) override;
    wxToolBarToolBase *FindToolForPosition(wxCoord x, wxCoord y) const override;

protected:
    // common part of all ctors
    void Init();

    
    bool DoInsertTool(size_t pos, wxToolBarToolBase *tool) override;
    bool DoDeleteTool(size_t pos, wxToolBarToolBase *tool) override;

    void DoEnableTool(wxToolBarToolBase *tool, bool enable) override;
    void DoToggleTool(wxToolBarToolBase *tool, bool toggle) override;
    void DoSetToggle(wxToolBarToolBase *tool, bool toggle) override;

    virtual wxToolBarToolBase *CreateTool(int id,
                                          const std::string& label,
                                          const wxBitmap& bmpNormal,
                                          const wxBitmap& bmpDisabled,
                                          wxItemKind kind,
                                          wxObject *clientData,
                                          const std::string& shortHelp,
                                          const std::string& longHelp) override;
    virtual wxToolBarToolBase *CreateTool(wxControl *control,
                                          const std::string& label) override;

    wxSize DoGetBestClientSize() const override;

    // calculate layout
    void DoLayout();

    // get the bounding rect for the given tool
    wxRect GetToolRect(wxToolBarToolBase *tool) const;

    // get the rect limits depending on the orientation: top/bottom for a
    // vertical toolbar, left/right for a horizontal one
    void GetRectLimits(const wxRect& rect, wxCoord *start, wxCoord *end) const;

    // receives button commands
    void OnCommand(wxCommandEvent& event);

    // paints a border
    void OnPaint(wxPaintEvent& event);

    // detects mouse clicks outside buttons
    void OnLeftUp(wxMouseEvent& event);

private:
    // have we calculated the positions of our tools?
    bool m_needsLayout;

    // the width of a separator
    wxCoord m_widthSeparator;

    // the total size of all toolbar elements
    wxCoord m_maxWidth,
            m_maxHeight;

    // the height of a label
    int m_labelHeight;

    // the space above the label
    int m_labelMargin;

private:
    wxDECLARE_EVENT_TABLE();
};

#endif
 // _WX_BUTTONBAR_H_

