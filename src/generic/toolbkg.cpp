///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/toolbkg.cpp
// Purpose:     generic implementation of wxToolbook
// Author:      Julian Smart
// Modified by:
// Created:     2006-01-29
// Copyright:   (c) 2006 Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_TOOLBOOK

#include "wx/icon.h"
#include "wx/toolbar.h"
#include "wx/imaglist.h"
#include "wx/toolbook.h"

#if defined(__WXMAC__) && wxUSE_TOOLBAR && wxUSE_BMPBUTTON
#include "wx/generic/buttonbar.h"
#endif

import WX.Utils.Settings;
import WX.Cmn.SysOpt;

// ----------------------------------------------------------------------------
// event table
// ----------------------------------------------------------------------------

wxDEFINE_EVENT( wxEVT_TOOLBOOK_PAGE_CHANGING, wxBookCtrlEvent );
wxDEFINE_EVENT( wxEVT_TOOLBOOK_PAGE_CHANGED,  wxBookCtrlEvent );

wxBEGIN_EVENT_TABLE(wxToolbook, wxBookCtrlBase)
    EVT_SIZE(wxToolbook::OnSize)
    EVT_TOOL(wxID_ANY, wxToolbook::OnToolSelected)
    EVT_IDLE(wxToolbook::OnIdle)
wxEND_EVENT_TABLE()

// ============================================================================
// wxToolbook implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxToolbook creation
// ----------------------------------------------------------------------------



bool wxToolbook::Create(wxWindow *parent,
                   wxWindowID id,
                   const wxPoint& pos,
                   const wxSize& size,
                   unsigned int style,
                   std::string_view name)
{
    if ( (style & wxBK_ALIGN_MASK) == wxBK_DEFAULT )
        style |= wxBK_TOP;

    // no border for this control
    style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    if ( !wxControl::Create(parent, id, pos, size, style,
                            wxValidator{}, name) )
        return false;

    int tbFlags = wxTB_TEXT | wxTB_FLAT | wxBORDER_NONE;
    if ( (style & (wxBK_LEFT | wxBK_RIGHT)) != 0 )
        tbFlags |= wxTB_VERTICAL;
    else
        tbFlags |= wxTB_HORIZONTAL;

    if ( style & wxTBK_HORZ_LAYOUT )
        tbFlags |= wxTB_HORZ_LAYOUT;

    // TODO: make more configurable

#if defined(__WXMAC__) && wxUSE_TOOLBAR && wxUSE_BMPBUTTON
    if (style & wxTBK_BUTTONBAR)
    {
        m_bookctrl = new wxButtonToolBar
                 (
                    this,
                    wxID_ANY,
                    wxDefaultPosition,
                    wxDefaultSize,
                    tbFlags
                 );
    }
    else
#endif
    {
        m_bookctrl = new wxToolBar
                 (
                    this,
                    wxID_ANY,
                    wxDefaultPosition,
                    wxDefaultSize,
                    tbFlags | wxTB_NODIVIDER
                 );
    }

    return true;
}

// ----------------------------------------------------------------------------
// wxToolbook geometry management
// ----------------------------------------------------------------------------

void wxToolbook::OnSize(wxSizeEvent& event)
{
    if (m_needsRealizing)
        Realize();

    wxBookCtrlBase::OnSize(event);
}

// ----------------------------------------------------------------------------
// accessing the pages
// ----------------------------------------------------------------------------

bool wxToolbook::SetPageText(size_t n, const std::string& strText)
{
    const int toolId = PageToToolId(n);
    wxToolBarToolBase* tool = GetToolBar()->FindById(toolId);
    if (tool)
    {
        tool->SetLabel(strText);
        return true;
    }
    else
        return false;
}

std::string wxToolbook::GetPageText(size_t n) const
{
    const int toolId = PageToToolId(n);
    wxToolBarToolBase* tool = GetToolBar()->FindById(toolId);
    if (tool)
        return tool->GetLabel();
    else
        return {};
}

int wxToolbook::GetPageImage([[maybe_unused]] size_t n) const
{
    wxFAIL_MSG( "wxToolbook::GetPageImage() not implemented" );

    return wxNOT_FOUND;
}

bool wxToolbook::SetPageImage(size_t n, int imageId)
{
    wxASSERT( GetImageList() != nullptr );
    if (!GetImageList())
        return false;

    const int toolId = PageToToolId(n);
    wxBitmap bmp = GetImageList()->GetBitmap(imageId);
    GetToolBar()->SetToolNormalBitmap(toolId, bmp);
    GetToolBar()->SetToolDisabledBitmap(toolId, bmp.ConvertToDisabled());

    return true;
}

// ----------------------------------------------------------------------------
// image list stuff
// ----------------------------------------------------------------------------

void wxToolbook::SetImageList(wxImageList *imageList)
{
    wxBookCtrlBase::SetImageList(imageList);
}

// ----------------------------------------------------------------------------
// selection
// ----------------------------------------------------------------------------

wxBookCtrlEvent* wxToolbook::CreatePageChangingEvent() const
{
    return new wxBookCtrlEvent(wxEVT_TOOLBOOK_PAGE_CHANGING, m_windowId);
}

void wxToolbook::MakeChangedEvent(wxBookCtrlEvent &event)
{
    event.SetEventType(wxEVT_TOOLBOOK_PAGE_CHANGED);
}

void wxToolbook::UpdateSelectedPage(size_t newsel)
{
    int toolId = PageToToolId(newsel);
    GetToolBar()->ToggleTool(toolId, true);
}

// Not part of the wxBookctrl API, but must be called in OnIdle or
// by application to realize the toolbar and select the initial page.
void wxToolbook::Realize()
{
    if (m_needsRealizing)
    {
        m_needsRealizing = false;

        GetToolBar()->SetToolBitmapSize(m_maxBitmapSize);

        GetToolBar()->Realize();
    }

    if (m_selection == wxNOT_FOUND)
        m_selection = 0;

    if (GetPageCount() > 0)
    {
        int sel = m_selection;
        m_selection = wxNOT_FOUND;
        SetSelection(sel);
    }

    DoSize();
}

int wxToolbook::HitTest(const wxPoint& pt, unsigned int* flags) const
{
    int pagePos = wxNOT_FOUND;

    if ( flags )
        *flags = wxBK_HITTEST_NOWHERE;

    // convert from wxToolbook coordinates to wxToolBar ones
    const wxToolBarBase * const tbar = GetToolBar();
    const wxPoint tbarPt = tbar->ScreenToClient(ClientToScreen(pt));

    // is the point over the toolbar?
    if ( wxRect(tbar->GetSize()).Contains(tbarPt) )
    {
        const wxToolBarToolBase * const
            tool = tbar->FindToolForPosition(tbarPt.x, tbarPt.y);

        if ( tool )
        {
            pagePos = tbar->GetToolPos(tool->GetId());
            if ( flags )
                *flags = wxBK_HITTEST_ONICON | wxBK_HITTEST_ONLABEL;
        }
    }
    else // not over the toolbar
    {
        if ( flags && GetPageRect().Contains(pt) )
            *flags |= wxBK_HITTEST_ONPAGE;
    }

    return pagePos;
}

void wxToolbook::OnIdle(wxIdleEvent& event)
{
    if (m_needsRealizing)
        Realize();
    event.Skip();
}

// ----------------------------------------------------------------------------
// adding/removing the pages
// ----------------------------------------------------------------------------

bool wxToolbook::InsertPage(size_t n,
                       wxWindow *page,
                       const std::string& text,
                       bool bSelect,
                       int imageId)
{
    if ( !wxBookCtrlBase::InsertPage(n, page, text, bSelect, imageId) )
        return false;

    m_needsRealizing = true;

    wxASSERT(GetImageList() != nullptr);

    if (!GetImageList())
        return false;

    // TODO: make sure all platforms can convert between icon and bitmap,
    // and/or test whether the image is a bitmap or an icon.
#ifdef __WXMAC__
    wxBitmap bitmap = GetImageList()->GetBitmap(imageId);
#else
    // On Windows, we can lose information by using GetBitmap, so extract icon instead
    wxIcon icon = GetImageList()->GetIcon(imageId);
    wxBitmap bitmap;
    bitmap.CopyFromIcon(icon);
#endif

    m_maxBitmapSize.x = std::max(bitmap.GetWidth(), m_maxBitmapSize.x);
    m_maxBitmapSize.y = std::max(bitmap.GetHeight(), m_maxBitmapSize.y);

    int toolId = page->GetId();
    GetToolBar()->SetToolBitmapSize(m_maxBitmapSize);
    GetToolBar()->InsertTool(n, toolId, text, bitmap, bitmap.ConvertToDisabled(), wxITEM_RADIO);

    // fix current selection
    if (m_selection == wxNOT_FOUND)
    {
        DoShowPage(page, true);
        m_selection = n;
    }
    else if ((size_t) m_selection >= n)
    {
        DoShowPage(page, false);
        m_selection++;
    }
    else
    {
        DoShowPage(page, false);
    }

    if ( bSelect )
    {
        SetSelection(n);
    }

    InvalidateBestSize();
    return true;
}

wxWindow *wxToolbook::DoRemovePage(size_t page)
{
    int toolId = PageToToolId(page);
    wxWindow *win = wxBookCtrlBase::DoRemovePage(page);

    if ( win )
    {
        GetToolBar()->DeleteTool(toolId);

        DoSetSelectionAfterRemoval(page);
    }

    return win;
}


bool wxToolbook::DeleteAllPages()
{
    GetToolBar()->ClearTools();
    return wxBookCtrlBase::DeleteAllPages();
}

bool wxToolbook::EnablePage(size_t page, bool enable)
{
    int toolId = PageToToolId(page);
    GetToolBar()->EnableTool(toolId, enable);
    if (!enable && GetSelection() == (int)page)
    {
        AdvanceSelection();
    }
    return true;
}

bool wxToolbook::EnablePage(wxWindow *page, bool enable)
{
    const int pageIndex = FindPage(page);
    if (pageIndex == wxNOT_FOUND)
    {
        return false;
    }
    return EnablePage(pageIndex, enable);
}

int wxToolbook::PageToToolId(size_t page) const
{
    wxCHECK_MSG(page < GetPageCount(), wxID_NONE, "Invalid page number");
    return GetPage(page)->GetId();
}

int wxToolbook::ToolIdToPage(int toolId) const
{
    for (size_t i = 0; i < m_pages.size(); i++)
    {
        if (m_pages[i]->GetId() == toolId)
        {
            return (int) i;
        }
    }
    return wxNOT_FOUND;
}

// ----------------------------------------------------------------------------
// wxToolbook events
// ----------------------------------------------------------------------------

void wxToolbook::OnToolSelected(wxCommandEvent& event)
{
    // find page for the tool
    const int page = ToolIdToPage(event.GetId());
    if (page == wxNOT_FOUND)
    {
        // this happens only of page id has changed afterwards
        return;
    }

    if (page == m_selection )
    {
        // this event can only come from our own Select(m_selection) below
        // which we call when the page change is vetoed, so we should simply
        // ignore it
        return;
    }

    SetSelection(page);

    // change wasn't allowed, return to previous state
    if (m_selection != page)
    {
        GetToolBar()->ToggleTool(m_selection, false);
    }
}

#endif // wxUSE_TOOLBOOK
