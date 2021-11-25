/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtextformatdlg.cpp
// Purpose:     Formatting dialog for wxRichTextCtrl
// Author:      Julian Smart
// Modified by:
// Created:     2006-10-01
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_RICHTEXT

#include "wx/richtext/richtextformatdlg.h"

#include "wx/listbox.h"
#include "wx/combobox.h"
#include "wx/textctrl.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include "wx/statline.h"
#include "wx/radiobut.h"
#include "wx/icon.h"
#include "wx/bitmap.h"
#include "wx/dcclient.h"
#include "wx/frame.h"
#include "wx/checkbox.h"
#include "wx/button.h"
#include "wx/wxcrtvararg.h"

#include "wx/bookctrl.h"
#include "wx/colordlg.h"
#include "wx/settings.h"
#include "wx/module.h"
#include "wx/imaglist.h"

#include "wx/richtext/richtextctrl.h"
#include "wx/richtext/richtextstyles.h"

#ifdef __WXMAC__
#include "../../src/richtext/richtextfontpage.cpp"
#include "../../src/richtext/richtextindentspage.cpp"
#include "../../src/richtext/richtexttabspage.cpp"
#include "../../src/richtext/richtextbulletspage.cpp"
#include "../../src/richtext/richtextstylepage.cpp"
#include "../../src/richtext/richtextliststylepage.cpp"
#include "../../src/richtext/richtextsizepage.cpp"
#include "../../src/richtext/richtextmarginspage.cpp"
#include "../../src/richtext/richtextborderspage.cpp"
#include "../../src/richtext/richtextbackgroundpage.cpp"
#else
#include "richtextfontpage.cpp"
#include "richtextindentspage.cpp"
#include "richtexttabspage.cpp"
#include "richtextbulletspage.cpp"
#include "richtextmarginspage.cpp"
#include "richtextsizepage.cpp"
#include "richtextborderspage.cpp"
#include "richtextbackgroundpage.cpp"
#include "richtextliststylepage.cpp"
#include "richtextstylepage.cpp"
#endif

#if 0 // def __WXMAC__
#define wxRICHTEXT_USE_TOOLBOOK 1
#else
#define wxRICHTEXT_USE_TOOLBOOK 0
#endif

wxIMPLEMENT_CLASS(wxRichTextDialogPage, wxPanel);

wxIMPLEMENT_CLASS(wxRichTextFormattingDialog, wxPropertySheetDialog);

wxBEGIN_EVENT_TABLE(wxRichTextFormattingDialog, wxPropertySheetDialog)
    EVT_BOOKCTRL_PAGE_CHANGED(wxID_ANY, wxRichTextFormattingDialog::OnTabChanged)
    EVT_BUTTON(wxID_HELP, wxRichTextFormattingDialog::OnHelp)
    EVT_UPDATE_UI(wxID_HELP, wxRichTextFormattingDialog::OnUpdateHelp)
wxEND_EVENT_TABLE()

IMPLEMENT_HELP_PROVISION(wxRichTextFormattingDialog)

wxRichTextFormattingDialog::~wxRichTextFormattingDialog()
{
    int sel = GetBookCtrl()->GetSelection();
    if (sel != -1 && sel < (int) m_pageIds.size())
        sm_lastPage = m_pageIds[sel];
}

bool wxRichTextFormattingDialog::Create(long flags, wxWindow* parent, const std::string& title, wxWindowID id,
        const wxPoint& pos, const wxSize& sz, unsigned int style)
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_BLOCK_EVENTS);
#ifdef __WXMAC__
    SetWindowVariant(wxWindowVariant::Small);
#endif

    GetFormattingDialogFactory()->SetSheetStyle(this);

    wxPropertySheetDialog::Create(parent, id, title, pos, sz, style | wxRESIZE_BORDER);

    GetFormattingDialogFactory()->CreateButtons(this);
    GetFormattingDialogFactory()->CreatePages(flags, this);

    LayoutDialog();

    if (sm_restoreLastPage && sm_lastPage != -1)
    {
        // FIXME: Improve this.
        auto idx = std::find(m_pageIds.begin(), m_pageIds.end(), sm_lastPage);
        if (idx != std::end(m_pageIds))
        {
            m_ignoreUpdates = true;
            GetBookCtrl()->SetSelection(std::distance(std::begin(m_pageIds), idx));
            m_ignoreUpdates = false;
        }
    }
    return true;
}

/// Get attributes from the given range
bool wxRichTextFormattingDialog::GetStyle(wxRichTextCtrl* ctrl, const wxRichTextRange& range)
{
    if (ctrl->GetFocusObject()->GetStyleForRange(range.ToInternal(), m_attributes))
        return UpdateDisplay();
    else
        return false;
}

/// Apply attributes to the given range, only applying if necessary (wxRICHTEXT_SETSTYLE_OPTIMIZE)
bool wxRichTextFormattingDialog::ApplyStyle(wxRichTextCtrl* ctrl, const wxRichTextRange& range, int flags)
{
    return ctrl->SetStyleEx(range, m_attributes, flags);
}

// Apply attributes to the object being edited, if any
bool wxRichTextFormattingDialog::ApplyStyle(wxRichTextCtrl* ctrl, int flags)
{
    if (wxGetObject())
    {
        ctrl->SetStyle(wxGetObject(), m_attributes, flags);
        return true;
    }
    else
        return false;
}

/// Set the attributes and optionally update the display
bool wxRichTextFormattingDialog::SetStyle(const wxRichTextAttr& style, bool update)
{
    m_attributes = style;
    if (update)
        UpdateDisplay();
    return true;
}

/// Set the style definition and optionally update the display
bool wxRichTextFormattingDialog::SetStyleDefinition(const wxRichTextStyleDefinition& styleDef, wxRichTextStyleSheet* sheet, bool update)
{
    m_styleSheet = sheet;

    m_styleDefinition = styleDef.Clone();

    return SetStyle(m_styleDefinition->GetStyle(), update);
}

/// Transfers the data and from to the window
bool wxRichTextFormattingDialog::TransferDataToWindow()
{
    if (m_styleDefinition)
        m_attributes = m_styleDefinition->GetStyle();

    return wxPropertySheetDialog::TransferDataToWindow();
}

bool wxRichTextFormattingDialog::TransferDataFromWindow()
{
    if (!wxPropertySheetDialog::TransferDataFromWindow())
        return false;

    if (m_styleDefinition)
        m_styleDefinition->GetStyle() = m_attributes;

    return true;
}

/// Update the display
bool wxRichTextFormattingDialog::UpdateDisplay()
{
    return TransferDataToWindow();
}

/// Apply the styles when a different tab is selected, so the previews are
/// up to date
void wxRichTextFormattingDialog::OnTabChanged(wxBookCtrlEvent& event)
{
    if (m_ignoreUpdates)
        return;

    if (GetBookCtrl() != event.GetEventObject())
    {
        event.Skip();
        return;
    }

    int oldPageId = event.GetOldSelection();
    if (oldPageId != -1)
    {
        wxWindow* page = GetBookCtrl()->GetPage(oldPageId);
        if (page)
            page->TransferDataFromWindow();
    }

    int pageId = event.GetSelection();
    if (pageId != -1)
    {
        wxWindow* page = GetBookCtrl()->GetPage(pageId);
        if (page)
            page->TransferDataToWindow();
    }
}

/// Respond to help command
void wxRichTextFormattingDialog::OnHelp(wxCommandEvent& event)
{
    int selPage = GetBookCtrl()->GetSelection();
    if (selPage != wxNOT_FOUND)
    {
        int pageId = -1;
        if (selPage < (int) m_pageIds.size())
            pageId = m_pageIds[selPage];
        if (!GetFormattingDialogFactory()->ShowHelp(pageId, this))
            event.Skip();
    }
}

void wxRichTextFormattingDialog::OnUpdateHelp(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void wxRichTextFormattingDialog::SetFormattingDialogFactory(std::unique_ptr<wxRichTextFormattingDialogFactory> factory)
{
    ms_FormattingDialogFactory = std::move(factory);
}

// Find a page by class
wxWindow* wxRichTextFormattingDialog::FindPage(wxClassInfo* info) const
{
    size_t i;
    for (i = 0; i < GetBookCtrl()->GetPageCount(); i++)
    {
        wxWindow* w = GetBookCtrl()->GetPage(i);
        if (w && w->wxGetClassInfo() == info)
            return w;
    }
    return nullptr;
}


/*!
 * Factory for formatting dialog
 */

/// Create all pages, under the dialog's book control, also calling AddPage
bool wxRichTextFormattingDialogFactory::CreatePages(long pages, wxRichTextFormattingDialog* dialog)
{
    if (dialog->GetImageList())
        dialog->GetBookCtrl()->SetImageList(dialog->GetImageList());

    int availablePageCount = GetPageIdCount();
    int i;
    bool selected = false;
    for (i = 0; i < availablePageCount; i ++)
    {
        int pageId = GetPageId(i);
        if (pageId != -1 && (pages & pageId))
        {
            std::string title;
            wxPanel* panel = CreatePage(pageId, title, dialog);
            wxASSERT( panel != nullptr );
            if (panel)
            {
                int imageIndex = GetPageImage(pageId);
                dialog->GetBookCtrl()->AddPage(panel, title, !selected, imageIndex);
                selected = true;

                dialog->AddPageId(pageId);
            }
        }
    }

    return true;
}

/// Create a page, given a page identifier
wxPanel* wxRichTextFormattingDialogFactory::CreatePage(int page, std::string& title, wxRichTextFormattingDialog* dialog)
{
    wxPanel* panel = nullptr;

    if (page == wxRICHTEXT_FORMAT_STYLE_EDITOR)
    {
        panel = new wxRichTextStylePage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Style");
    }
    else if (page == wxRICHTEXT_FORMAT_FONT)
    {
        panel = new wxRichTextFontPage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Font");
    }
    else if (page == wxRICHTEXT_FORMAT_INDENTS_SPACING)
    {
        panel = new wxRichTextIndentsSpacingPage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Indents && Spacing");
    }
    else if (page == wxRICHTEXT_FORMAT_TABS)
    {
        panel = new wxRichTextTabsPage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Tabs");
    }
    else if (page == wxRICHTEXT_FORMAT_BULLETS)
    {
        panel = new wxRichTextBulletsPage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Bullets");
    }
    else if (page == wxRICHTEXT_FORMAT_LIST_STYLE)
    {
        panel = new wxRichTextListStylePage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("List Style");
    }
    else if (page == wxRICHTEXT_FORMAT_SIZE)
    {
        panel = new wxRichTextSizePage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Size");
    }
    else if (page == wxRICHTEXT_FORMAT_MARGINS)
    {
        panel = new wxRichTextMarginsPage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Margins");
    }
    else if (page == wxRICHTEXT_FORMAT_BORDERS)
    {
        panel = new wxRichTextBordersPage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Borders");
    }
    else if (page == wxRICHTEXT_FORMAT_BACKGROUND)
    {
        panel = new wxRichTextBackgroundPage(dialog->GetBookCtrl(), wxID_ANY);
        title = _("Background");
    }

    return panel;
}

/// Enumerate all available page identifiers
int wxRichTextFormattingDialogFactory::GetPageId(int i) const
{
    int pages[] = {
        wxRICHTEXT_FORMAT_STYLE_EDITOR,
        wxRICHTEXT_FORMAT_FONT,
        wxRICHTEXT_FORMAT_INDENTS_SPACING,
        wxRICHTEXT_FORMAT_BULLETS,
        wxRICHTEXT_FORMAT_TABS,
        wxRICHTEXT_FORMAT_LIST_STYLE,
        wxRICHTEXT_FORMAT_SIZE,
        wxRICHTEXT_FORMAT_MARGINS,
        wxRICHTEXT_FORMAT_BORDERS,
        wxRICHTEXT_FORMAT_BACKGROUND
        };

    if (i < 0 || i >= GetPageIdCount())
        return -1;

    return pages[i];
}

/// Get the number of available page identifiers
int wxRichTextFormattingDialogFactory::GetPageIdCount() const
{
    return 10;
}

/// Set the sheet style, called at the start of wxRichTextFormattingDialog::Create
bool wxRichTextFormattingDialogFactory::SetSheetStyle(wxRichTextFormattingDialog* dialog)
{
#if wxRICHTEXT_USE_TOOLBOOK
    int sheetStyle = wxPROPSHEET_SHRINKTOFIT;
#ifdef __WXMAC__
    sheetStyle |= wxPROPSHEET_BUTTONTOOLBOOK;
#else
    sheetStyle |= wxPROPSHEET_TOOLBOOK;
#endif

    dialog->SetSheetStyle(sheetStyle);
    dialog->SetSheetInnerBorder(0);
    dialog->SetSheetOuterBorder(0);
#else
    wxUnusedVar(dialog);
#endif // wxRICHTEXT_USE_TOOLBOOK

    return true;
}

/// Create the main dialog buttons
bool wxRichTextFormattingDialogFactory::CreateButtons(wxRichTextFormattingDialog* dialog)
{
    int flags = wxOK|wxCANCEL;
    if (dialog->GetWindowStyleFlag() & wxRICHTEXT_FORMAT_HELP_BUTTON)
        flags |= wxHELP;

    // If using a toolbook, also follow Mac style and don't create buttons
#if !wxRICHTEXT_USE_TOOLBOOK
    dialog->CreateButtons(flags);
#endif

    return true;
}

// Invoke help for the dialog
bool wxRichTextFormattingDialogFactory::ShowHelp([[maybe_unused]] int page, wxRichTextFormattingDialog* dialog)
{
    wxRichTextDialogPage* window = nullptr;
    int sel = dialog->GetBookCtrl()->GetSelection();
    if (sel != -1)
        window = wxDynamicCast(dialog->GetBookCtrl()->GetPage(sel), wxRichTextDialogPage);
    if (window && window->GetHelpId() != -1)
    {
        if (window->GetUICustomization())
            return window->GetUICustomization()->ShowHelp(dialog, window->GetHelpId());
        else if (dialog->GetUICustomization())
            return dialog->GetUICustomization()->ShowHelp(dialog, window->GetHelpId());
        else
            return false;
    }
    else if (dialog->GetHelpId() != -1 && dialog->GetUICustomization())
        return dialog->ShowHelp(dialog);
    else
        return false;
}

/*
 * Module to initialise and clean up handlers
 */

class wxRichTextFormattingDialogModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextFormattingDialogModule);
public:
    bool OnInit() override { wxRichTextFormattingDialog::SetFormattingDialogFactory(std::make_unique<wxRichTextFormattingDialogFactory>()); return true; }
    void OnExit() override { wxRichTextFormattingDialog::SetFormattingDialogFactory(nullptr); }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextFormattingDialogModule, wxModule);

/*
 * Font preview control
 */

wxBEGIN_EVENT_TABLE(wxRichTextFontPreviewCtrl, wxWindow)
    EVT_PAINT(wxRichTextFontPreviewCtrl::OnPaint)
wxEND_EVENT_TABLE()

wxRichTextFontPreviewCtrl::wxRichTextFontPreviewCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& sz, unsigned int style)
{
    if ((style & wxBORDER_MASK) == wxBORDER_DEFAULT)
        style |= wxBORDER_THEME;

    wxWindow::Create(parent, id, pos, sz, style);

    SetBackgroundColour(*wxWHITE);
    m_textEffects = 0;
}

void wxRichTextFontPreviewCtrl::OnPaint([[maybe_unused]] wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxSize size = GetSize();
    wxFont font = GetFont();

    if ((GetTextEffects() & wxTEXT_ATTR_EFFECT_SUPERSCRIPT) || (GetTextEffects() & wxTEXT_ATTR_EFFECT_SUBSCRIPT))
    {
        font.SetFractionalPointSize(font.GetFractionalPointSize() / wxSCRIPT_MUL_FACTOR);
    }

    if ( font.IsOk() )
    {
        dc.SetFont(font);
        // Calculate vertical and horizontal centre
        wxCoord w = 0, h = 0;

        std::string text(_("ABCDEFGabcdefg12345").ToStdString());
        if (GetTextEffects() & wxTEXT_ATTR_EFFECT_CAPITALS)
            wx::utils::ToUpper(text);

        dc.GetTextExtent( text, &w, &h);
        int cx = std::max(2, (size.x/2) - (w/2));
        int cy = std::max(2, (size.y/2) - (h/2));

        if ( GetTextEffects() & wxTEXT_ATTR_EFFECT_SUPERSCRIPT )
            cy -= h/2;
        if ( GetTextEffects() & wxTEXT_ATTR_EFFECT_SUBSCRIPT )
            cy += h/2;

        dc.SetTextForeground(GetForegroundColour());
        dc.SetClippingRegion(2, 2, size.x-4, size.y-4);
        dc.wxDrawText(text, wxPoint{cx, cy});

        if (GetTextEffects() & wxTEXT_ATTR_EFFECT_STRIKETHROUGH)
        {
            dc.SetPen(wxPen(GetForegroundColour(), 1));
            dc.DrawLine(cx, (int) (cy + h/2 + 0.5), cx + w, (int) (cy + h/2 + 0.5));
        }

        dc.DestroyClippingRegion();
    }
}

// Helper for pages to get the top-level dialog
wxRichTextFormattingDialog* wxRichTextFormattingDialog::GetDialog(wxWindow* win)
{
    wxWindow* p = win->GetParent();
    while (p && !wxDynamicCast(p, wxRichTextFormattingDialog))
        p = p->GetParent();
    wxRichTextFormattingDialog* dialog = wxDynamicCast(p, wxRichTextFormattingDialog);
    return dialog;
}

// Helper for pages to get the attributes
wxRichTextAttr* wxRichTextFormattingDialog::GetDialogAttributes(wxWindow* win)
{
    wxRichTextFormattingDialog* dialog = GetDialog(win);
    if (dialog)
        return & dialog->GetAttributes();
    else
        return nullptr;
}

#if 0
// Helper for pages to get the attributes to reset
wxRichTextAttr* wxRichTextFormattingDialog::GetDialogResetAttributes(wxWindow* win)
{
    wxRichTextFormattingDialog* dialog = GetDialog(win);
    if (dialog)
        return & dialog->GetResetAttributes();
    else
        return nullptr;
}
#endif

// Helper for pages to get the style
wxRichTextStyleDefinition* wxRichTextFormattingDialog::GetDialogStyleDefinition(wxWindow* win)
{
    wxRichTextFormattingDialog* dialog = GetDialog(win);
    if (dialog)
        return dialog->GetStyleDefinition();
    else
        return nullptr;
}

void wxRichTextFormattingDialog::SetDimensionValue(wxTextAttrDimension& dim, wxTextCtrl* valueCtrl, wxComboBox* unitsCtrl, wxCheckBox* checkBox, std::vector<int>* units)
{
    if (!dim.IsValid())
    {
        if (checkBox)
            checkBox->SetValue(false);
        valueCtrl->SetValue("0");
        if (unitsCtrl)
            unitsCtrl->SetSelection(0);
    }
    else
    {
        auto unitsIdx = units->begin();

        if (checkBox)
            checkBox->SetValue(true);
        
        if (dim.GetUnits() == wxTEXT_ATTR_UNITS_PIXELS)
        {
            // By default, the 1st in the list.
            valueCtrl->SetValue(fmt::format("{:d}", (int) dim.GetValue()));
        }
        else if (dim.GetUnits() == wxTEXT_ATTR_UNITS_TENTHS_MM)
        {
            unitsIdx += 1; // By default, the 2nd in the list.
            double value = dim.GetValue() / 100.0;
            valueCtrl->SetValue(fmt::format("{:.2f}", value));
        }
        else if (dim.GetUnits() == wxTEXT_ATTR_UNITS_PERCENTAGE)
        {
            unitsIdx += 2; // By default, the 3rd in the list.
            valueCtrl->SetValue(fmt::format("{:d}", (int) dim.GetValue()));
        }
        else if (dim.GetUnits() == wxTEXT_ATTR_UNITS_HUNDREDTHS_POINT)
        {
            unitsIdx += 3; // By default, the 4th in the list.
            double value = dim.GetValue() / 100.0;
            valueCtrl->SetValue(fmt::format("{:.2f}", value));
        }
        else if (dim.GetUnits() == wxTEXT_ATTR_UNITS_POINTS)
        {
            unitsIdx += 3; // By default, the 4th in the list (we don't have points and hundredths of points in the same list)
            valueCtrl->SetValue(fmt::format("{:d}", (int) dim.GetValue()));
        }
        
        if (units)
        {
            unitsIdx = std::find(units->begin(), units->end(), dim.GetUnits());
            if (unitsIdx == std::end(*units))
                unitsIdx = std::begin(*units);
        }

        if (unitsCtrl)
            unitsCtrl->SetSelection(std::distance(std::begin(*units), unitsIdx));
    }
}

void wxRichTextFormattingDialog::GetDimensionValue(wxTextAttrDimension& dim, wxTextCtrl* valueCtrl, wxComboBox* unitsCtrl, wxCheckBox* checkBox, std::vector<int>* units)
{
    int unitsSel = 0;
    if (unitsCtrl)
        unitsSel = unitsCtrl->GetSelection();

    if (checkBox && !checkBox->GetValue())
    {
        dim.Reset();
    }
    else
    {
        if (units)
        {
            int unit = (*units)[unitsSel];
            dim.SetUnits((wxTextAttrUnits) unit);
        }
        else
        {
            if (unitsSel == 0)
                dim.SetUnits(wxTEXT_ATTR_UNITS_PIXELS);
            else if (unitsSel == 1)
                dim.SetUnits(wxTEXT_ATTR_UNITS_TENTHS_MM);
            else if (unitsSel == 2)
                dim.SetUnits(wxTEXT_ATTR_UNITS_PERCENTAGE);
            else if (unitsSel == 3)
                dim.SetUnits(wxTEXT_ATTR_UNITS_HUNDREDTHS_POINT);
        }

        int value = 0;
        if (ConvertFromString(valueCtrl->GetValue(), value, dim.GetUnits()))
            dim.SetValue(value);
    }
}

bool wxRichTextFormattingDialog::ConvertFromString(const std::string& str, int& ret, int unit)
{
    if (unit == wxTEXT_ATTR_UNITS_PIXELS)
    {
        ret = wxAtoi(str);
        return true;
    }
    else if (unit == wxTEXT_ATTR_UNITS_TENTHS_MM)
    {
        float value = 0;
        wxSscanf(str.c_str(), "%f", &value);
        // Convert from cm
        // Do this in two steps, since using one step causes strange rounding error for VS 2010 at least.
        float v = value * 100;
        ret = (int) (v);
        return true;
    }
    else if (unit == wxTEXT_ATTR_UNITS_PERCENTAGE)
    {
        ret = wxAtoi(str);
        return true;
    }
    else if (unit == wxTEXT_ATTR_UNITS_HUNDREDTHS_POINT)
    {
        float value = 0;
        wxSscanf(str.c_str(), "%f", &value);
        float v = value * 100;
        ret = (int) (v);
    }
    else if (unit == wxTEXT_ATTR_UNITS_POINTS)
    {
        ret = wxAtoi(str);
        return true;
    }
    else
    {
        ret = 0;
        return false;
    }

    return true;
}

/*
 * A control for displaying a small preview of a colour or bitmap
 */

wxBEGIN_EVENT_TABLE(wxRichTextColourSwatchCtrl, wxControl)
    EVT_MOUSE_EVENTS(wxRichTextColourSwatchCtrl::OnMouseEvent)
wxEND_EVENT_TABLE()

wxIMPLEMENT_CLASS(wxRichTextColourSwatchCtrl, wxControl);

wxRichTextColourSwatchCtrl::wxRichTextColourSwatchCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style)
{
    if ((style & wxBORDER_MASK) == wxBORDER_DEFAULT)
        style |= wxBORDER_THEME;

    wxControl::Create(parent, id, pos, size, style);

    SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
}

void wxRichTextColourSwatchCtrl::OnMouseEvent(wxMouseEvent& event)
{
    if (event.LeftDown())
    {
        wxWindow* parent = GetParent();
        while (parent != nullptr && !wxDynamicCast(parent, wxDialog) && !wxDynamicCast(parent, wxFrame))
            parent = parent->GetParent();

        wxRichTextFormattingDialog* dlg = wxDynamicCast(parent, wxRichTextFormattingDialog);
        wxColourData data;
        if (dlg)
            data = wxRichTextFormattingDialog::GetColourData();

        data.SetChooseFull(true);
        data.SetColour(m_colour);
#if wxUSE_COLOURDLG
        wxColourDialog *dialog = new wxColourDialog(parent, &data);
        // Crashes on wxMac (no m_peer)
#ifndef __WXMAC__
        dialog->SetTitle(_("Colour"));
#endif
        if (dialog->ShowModal() == wxID_OK)
        {
            wxColourData retData = dialog->GetColourData();
            if (dlg)
                wxRichTextFormattingDialog::SetColourData(retData);
            m_colour = retData.GetColour();
            SetBackgroundColour(m_colour);
        }
        dialog->Destroy();
#endif // wxUSE_COLOURDLG
        Refresh();

        wxCommandEvent btnEvent(wxEVT_BUTTON, GetId());
        GetEventHandler()->ProcessEvent(btnEvent);
    }
}

#if wxUSE_HTML

/*!
 * wxRichTextFontListBox class declaration
 * A listbox to display styles.
 */

wxIMPLEMENT_CLASS(wxRichTextFontListBox, wxHtmlListBox);

wxBEGIN_EVENT_TABLE(wxRichTextFontListBox, wxHtmlListBox)
wxEND_EVENT_TABLE()

wxRichTextFontListBox::wxRichTextFontListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, unsigned int style)
{
    Create(parent, id, pos, size, style);
}

bool wxRichTextFontListBox::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, unsigned int style)
{
    if ((style & wxBORDER_MASK) == wxBORDER_DEFAULT)
        style |= wxBORDER_THEME;

    return wxHtmlListBox::Create(parent, id, pos, size, style);
}

/// Returns the HTML for this item
std::string wxRichTextFontListBox::OnGetItem(size_t n) const
{
    if (m_faceNames.size() == 0)
        return {};

    return CreateHTML(m_faceNames[n]);
}

/// Get font name for index
std::string wxRichTextFontListBox::GetFaceName(size_t i) const
{
    return m_faceNames[i];
}

/// Set selection for string, returning the index.
int wxRichTextFontListBox::SetFaceNameSelection(const std::string& name)
{
    // FIXME: What if nothing is found?
    // FIXME: Case sensitivity, does it matter here?
    int i = std::distance(std::cbegin(m_faceNames), std::find(m_faceNames.cbegin(), m_faceNames.cend(), name));

    SetSelection(i);

    return i;
}

/// Updates the font list
void wxRichTextFontListBox::UpdateFonts()
{
    std::vector<std::string> facenames = wxRichTextCtrl::GetAvailableFontNames();
    m_faceNames = facenames;
    std::sort(m_faceNames.begin(), m_faceNames.end());

    SetItemCount(m_faceNames.size());
    Refresh();
}

#if 0
// Convert a colour to a 6-digit hex string
static std::string ColourToHexString(const wxColour& col)
{
    std::string hex;

    hex += wxDecToHex(col.Red());
    hex += wxDecToHex(col.Green());
    hex += wxDecToHex(col.Blue());

    return hex;
}
#endif

/// Creates a suitable HTML fragment for a definition
std::string wxRichTextFontListBox::CreateHTML(const std::string& facename) const
{
    std::string str = "<font";

    str += " size=\"+2\"";

    if (!facename.empty() && facename != _("(none)").ToStdString())
        str += " face=\"" + facename + "\"";
/*
    if (def->GetStyle().GetTextColour().IsOk())
        str << " color=\"#") << ColourToHexString(def->GetStyle().GetTextColour()) << wxT("\"";
*/

    str += ">";

    bool hasBold = false;

    if (hasBold)
        str += "<b>";

    str += facename;

    if (hasBold)
        str += "</b>";

    str += "</font>";

    return str;
}

#endif
    // wxUSE_HTML


#endif
    // wxUSE_RICHTEXT
