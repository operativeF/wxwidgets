/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/stattextg.cpp
// Purpose:     wxGenericStaticText
// Author:      Marcin Wojdyr
// Created:     2008-06-26
// Copyright:   Marcin Wojdyr
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STATTEXT

#include "wx/dcclient.h"
#include "wx/settings.h"
#include "wx/validate.h"

#include "wx/generic/stattextg.h"

#if wxUSE_MARKUP
    #include "wx/generic/private/markuptext.h"
#endif // wxUSE_MARKUP

wxIMPLEMENT_DYNAMIC_CLASS(wxGenericStaticText, wxStaticTextBase);


bool wxGenericStaticText::Create(wxWindow *parent,
                                 wxWindowID id,
                                 const std::string &label,
                                 const wxPoint &pos,
                                 const wxSize &size,
                                 unsigned int style,
                                 std::string_view name)
{
    if ( !wxControl::Create(parent, id, pos, size, style,
                            wxDefaultValidator, name) )
        return false;

    SetLabel(label);
    SetInitialSize(size);
    Bind(wxEVT_PAINT, &wxGenericStaticText::OnPaint, this);
    return true;
}

wxGenericStaticText::~wxGenericStaticText()
{
#if wxUSE_MARKUP
    delete m_markupText;
#endif // wxUSE_MARKUP
}

void wxGenericStaticText::DoDrawLabel(wxDC& dc, const wxRect& rect)
{
#if wxUSE_MARKUP
    if ( m_markupText )
        m_markupText->Render(dc, rect, wxMarkupText::Render_ShowAccels);
    else
#endif // wxUSE_MARKUP
        dc.DrawLabel(m_label, rect, GetAlignment(), m_mnemonic);
}

void wxGenericStaticText::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    wxRect rect = GetClientRect();
    if ( !IsEnabled() )
    {
        // draw shadow of the text
        dc.SetTextForeground(
                       wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
        wxRect rectShadow = rect;
        rectShadow.Offset(1, 1);
        DoDrawLabel(dc, rectShadow);
        dc.SetTextForeground(
                       wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW));
    }
    DoDrawLabel(dc, rect);
}


wxSize wxGenericStaticText::DoGetBestClientSize() const
{
    wxClientDC dc(const_cast<wxGenericStaticText *>(this));

#if wxUSE_MARKUP
    if ( m_markupText )
        return m_markupText->Measure(dc);
#endif // wxUSE_MARKUP

    return dc.GetMultiLineTextExtent(GetLabel());
}

void wxGenericStaticText::SetLabel(std::string_view label)
{
    wxControl::SetLabel(label);
    WXSetVisibleLabel(GetEllipsizedLabel());

    AutoResizeIfNecessary();

#if wxUSE_MARKUP
    if ( m_markupText )
    {
        delete m_markupText;
        m_markupText = nullptr;
    }
#endif // wxUSE_MARKUP

    Refresh();
}

void wxGenericStaticText::WXSetVisibleLabel(const std::string& label)
{
    m_mnemonic = FindAccelIndex(label, &m_label);
}

#if wxUSE_MARKUP

bool wxGenericStaticText::DoSetLabelMarkup(const std::string& markup)
{
    if ( !wxStaticTextBase::DoSetLabelMarkup(markup) )
        return false;

    if ( !m_markupText )
        m_markupText = new wxMarkupText(markup);
    else
        m_markupText->SetMarkup(markup);

    AutoResizeIfNecessary();

    Refresh();

    return true;
}

#endif // wxUSE_MARKUP

bool wxGenericStaticText::SetFont(const wxFont &font)
{
    if ( !wxControl::SetFont(font) )
        return false;

    AutoResizeIfNecessary();

    Refresh();
    return true;
}

void wxGenericStaticText::DoSetSize(wxRect boundary,
                                    unsigned int sizeFlags)
{
    wxStaticTextBase::DoSetSize(boundary, sizeFlags);
    UpdateLabel();
}


#endif // wxUSE_STATTEXT
