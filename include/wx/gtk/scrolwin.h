/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/scrolwin.h
// Purpose:     wxScrolledWindow class
// Author:      Robert Roebling
// Modified by: Vadim Zeitlin (2005-10-10): wxScrolledWindow is now common
// Created:     01/02/97
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_SCROLLWIN_H_
#define _WX_GTK_SCROLLWIN_H_

// ----------------------------------------------------------------------------
// wxScrolledWindow
// ----------------------------------------------------------------------------

class wxScrollHelper : public wxScrollHelperBase
{
    typedef wxScrollHelperBase base_type;
public:
    // default ctor doesn't do anything
    wxScrollHelper(wxWindow *win) : wxScrollHelperBase(win) { }

    // implement the base class methods
    virtual void SetScrollbars(int pixelsPerUnitX, int pixelsPerUnitY,
                               int noUnitsX, int noUnitsY,
                               int xPos = 0, int yPos = 0,
                               bool noRefresh = false) override;
    void AdjustScrollbars() override;

    bool IsScrollbarShown(int orient) const override;

protected:
    void DoScroll(int x, int y) override;
    virtual void DoShowScrollbars(wxScrollbarVisibility horz,
                                  wxScrollbarVisibility vert) override;

private:
    // this does (each) half of AdjustScrollbars() work
    void DoAdjustScrollbar(GtkRange* range,
                           int pixelsPerLine,
                           int winSize,
                           int virtSize,
                           int *pos,
                           int *lines,
                           int *linesPerPage);

    void DoAdjustHScrollbar(int winSize, int virtSize)
    {
        DoAdjustScrollbar
        (
            m_win->m_scrollBar[wxWindow::ScrollDir_Horz],
            m_xScrollPixelsPerLine, winSize, virtSize,
            &m_xScrollPosition, &m_xScrollLines, &m_xScrollLinesPerPage
        );
    }

    void DoAdjustVScrollbar(int winSize, int virtSize)
    {
        DoAdjustScrollbar
        (
            m_win->m_scrollBar[wxWindow::ScrollDir_Vert],
            m_yScrollPixelsPerLine, winSize, virtSize,
            &m_yScrollPosition, &m_yScrollLines, &m_yScrollLinesPerPage
        );
    }

    // and this does the same for Scroll()
    void DoScrollOneDir(int orient,
                        int pos,
                        int pixelsPerLine,
                        int *posOld);

    wxScrollHelper(const wxScrollHelper&) = delete;
	wxScrollHelper& operator=(const wxScrollHelper&) = delete;
};

#endif // _WX_GTK_SCROLLWIN_H_

