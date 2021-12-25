/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtextstyles.cpp
// Purpose:     Style management for wxRichTextCtrl
// Author:      Julian Smart
// Modified by:
// Created:     2005-09-30
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_RICHTEXT

#include "wx/richtext/richtextstyles.h"

#include "wx/filename.h"
#include "wx/clipbrd.h"
#include "wx/dc.h"
#include "wx/dcclient.h"

#include "wx/richtext/richtextctrl.h"

import WX.Utils.Settings;
import WX.Core.Sizer;
import WX.Cmn.WFStream;

import WX.Cfg.Flags;

wxIMPLEMENT_CLASS(wxRichTextStyleDefinition, wxObject);
wxIMPLEMENT_CLASS(wxRichTextCharacterStyleDefinition, wxRichTextStyleDefinition);
wxIMPLEMENT_CLASS(wxRichTextParagraphStyleDefinition, wxRichTextStyleDefinition);
wxIMPLEMENT_CLASS(wxRichTextListStyleDefinition, wxRichTextParagraphStyleDefinition);
wxIMPLEMENT_CLASS(wxRichTextBoxStyleDefinition, wxRichTextStyleDefinition);

/*!
 * A definition
 */

void wxRichTextStyleDefinition::Copy(const wxRichTextStyleDefinition& def)
{
    m_name = def.m_name;
    m_baseStyle = def.m_baseStyle;
    m_style = def.m_style;
    m_description = def.m_description;
    m_properties = def.m_properties;
}

bool wxRichTextStyleDefinition::Eq(const wxRichTextStyleDefinition& def) const
{
    return (m_name == def.m_name && m_baseStyle == def.m_baseStyle && m_style == def.m_style && m_properties == def.m_properties);
}

/// Gets the style combined with the base style
wxRichTextAttr wxRichTextStyleDefinition::GetStyleMergedWithBase(const wxRichTextStyleSheet* sheet) const
{
    if (m_baseStyle.empty())
        return m_style;

    bool isParaStyle = IsKindOf(wxCLASSINFO(wxRichTextParagraphStyleDefinition));
    bool isCharStyle = IsKindOf(wxCLASSINFO(wxRichTextCharacterStyleDefinition));
    bool isListStyle = IsKindOf(wxCLASSINFO(wxRichTextListStyleDefinition));
    bool isBoxStyle  = IsKindOf(wxCLASSINFO(wxRichTextBoxStyleDefinition));

    // Collect the styles, detecting loops
    std::vector<wxString> styleNames;
    wxList styles;
    const wxRichTextStyleDefinition* def = this;
    while (def)
    {
        styles.Insert(const_cast<wxRichTextStyleDefinition*>(def));
        styleNames.Add(def->GetName());

        wxString baseStyleName = def->GetBaseStyle();
        if (!baseStyleName.IsEmpty() && styleNames.Index(baseStyleName) == wxNOT_FOUND)
        {
            if (isParaStyle)
                def = sheet->FindParagraphStyle(baseStyleName);
            else if (isCharStyle)
                def = sheet->FindCharacterStyle(baseStyleName);
            else if (isListStyle)
                def = sheet->FindListStyle(baseStyleName);
            else if (isBoxStyle)
                def = sheet->FindBoxStyle(baseStyleName);
            else
                def = sheet->FindStyle(baseStyleName);
        }
        else
            def = nullptr;
    }

    wxRichTextAttr attr;
    wxList::compatibility_iterator node = styles.GetFirst();
    while (node)
    {
        wxRichTextStyleDefinition* nodeDef = (wxRichTextStyleDefinition*) node->GetData();
        attr.Apply(nodeDef->GetStyle(), nullptr);
        node = node->GetNext();
    }

    return attr;
}

/*!
 * Paragraph style definition
 */

void wxRichTextParagraphStyleDefinition::Copy(const wxRichTextParagraphStyleDefinition& def)
{
    wxRichTextStyleDefinition::Copy(def);

    m_nextStyle = def.m_nextStyle;
}

bool wxRichTextParagraphStyleDefinition::operator ==(const wxRichTextParagraphStyleDefinition& def) const
{
    return (Eq(def) && m_nextStyle == def.m_nextStyle);
}

/*!
 * Box style definition
 */

void wxRichTextBoxStyleDefinition::Copy(const wxRichTextBoxStyleDefinition& def)
{
    wxRichTextStyleDefinition::Copy(def);
}

bool wxRichTextBoxStyleDefinition::operator ==(const wxRichTextBoxStyleDefinition& def) const
{
    return (Eq(def));
}

/*!
 * List style definition
 */

void wxRichTextListStyleDefinition::Copy(const wxRichTextListStyleDefinition& def)
{
    wxRichTextParagraphStyleDefinition::Copy(def);

    int i;
    for (i = 0; i < 10; i++)
        m_levelStyles[i] = def.m_levelStyles[i];
}

bool wxRichTextListStyleDefinition::operator ==(const wxRichTextListStyleDefinition& def) const
{
    if (!Eq(def))
        return false;
    int i;
    for (i = 0; i < 10; i++)
        if (!(m_levelStyles[i] == def.m_levelStyles[i]))
            return false;

    return true;
}

/// Sets/gets the attributes for the given level
void wxRichTextListStyleDefinition::SetLevelAttributes(int i, const wxRichTextAttr& attr)
{
    wxASSERT( (i >= 0 && i < 10) );
    if (i >= 0 && i < 10)
        m_levelStyles[i] = attr;
}

const wxRichTextAttr* wxRichTextListStyleDefinition::GetLevelAttributes(int i) const
{
    wxASSERT( (i >= 0 && i < 10) );
    if (i >= 0 && i < 10)
        return & m_levelStyles[i];
    else
        return nullptr;
}

wxRichTextAttr* wxRichTextListStyleDefinition::GetLevelAttributes(int i)
{
    wxASSERT( (i >= 0 && i < 10) );
    if (i >= 0 && i < 10)
        return & m_levelStyles[i];
    else
        return nullptr;
}

/// Convenience function for setting the major attributes for a list level specification
void wxRichTextListStyleDefinition::SetAttributes(int i, int leftIndent, int leftSubIndent, int bulletStyle, const std::string& bulletSymbol)
{
    wxASSERT( (i >= 0 && i < 10) );
    if (i >= 0 && i < 10)
    {
        wxRichTextAttr attr;

        attr.SetBulletStyle(bulletStyle);
        attr.SetLeftIndent(leftIndent, leftSubIndent);

        if (!bulletSymbol.empty())
        {
            if (bulletStyle & wxTEXT_ATTR_BULLET_STYLE_SYMBOL)
                attr.SetBulletText(bulletSymbol);
            else
                attr.SetBulletName(bulletSymbol);
        }

        m_levelStyles[i] = attr;
    }
}

/// Finds the level corresponding to the given indentation
int wxRichTextListStyleDefinition::FindLevelForIndent(int indent) const
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (indent < m_levelStyles[i].GetLeftIndent())
        {
            if (i > 0)
                return i - 1;
            else
                return 0;
        }
    }
    return 9;
}

/// Combine the list style with a paragraph style, using the given indent (from which
/// an appropriate level is found)
wxRichTextAttr wxRichTextListStyleDefinition::CombineWithParagraphStyle(int indent, const wxRichTextAttr& paraStyle, wxRichTextStyleSheet* styleSheet)
{
    int listLevel = FindLevelForIndent(indent);

    wxRichTextAttr attr(*GetLevelAttributes(listLevel));
    int oldLeftIndent = attr.GetLeftIndent();
    int oldLeftSubIndent = attr.GetLeftSubIndent();

    // First apply the overall paragraph style, if any
    if (styleSheet)
        attr.Apply(GetStyleMergedWithBase(styleSheet));
    else
        attr.Apply(GetStyle());

    // Then apply paragraph style, e.g. from paragraph style definition
    attr.Apply(paraStyle);

    // We override the indents according to the list definition
    attr.SetLeftIndent(oldLeftIndent, oldLeftSubIndent);

    return attr;
}

/// Combine the base and list style, using the given indent (from which
/// an appropriate level is found)
wxRichTextAttr wxRichTextListStyleDefinition::GetCombinedStyle(int indent, wxRichTextStyleSheet* styleSheet)
{
    int listLevel = FindLevelForIndent(indent);
    return GetCombinedStyleForLevel(listLevel, styleSheet);
}

/// Combine the base and list style, using the given indent (from which
/// an appropriate level is found)
wxRichTextAttr wxRichTextListStyleDefinition::GetCombinedStyleForLevel(int listLevel, wxRichTextStyleSheet* styleSheet)
{
    wxRichTextAttr attr(*GetLevelAttributes(listLevel));
    int oldLeftIndent = attr.GetLeftIndent();
    int oldLeftSubIndent = attr.GetLeftSubIndent();

    // Apply the overall paragraph style, if any
    if (styleSheet)
        attr.Apply(GetStyleMergedWithBase(styleSheet));
    else
        attr.Apply(GetStyle());

    // We override the indents according to the list definition
    attr.SetLeftIndent(oldLeftIndent, oldLeftSubIndent);

    return attr;
}

/// Is this a numbered list?
bool wxRichTextListStyleDefinition::IsNumbered(int i) const
{
    return (0 != (GetLevelAttributes(i)->GetFlags() &
                   (wxTEXT_ATTR_BULLET_STYLE_ARABIC|wxTEXT_ATTR_BULLET_STYLE_LETTERS_UPPER|wxTEXT_ATTR_BULLET_STYLE_LETTERS_LOWER|
                    wxTEXT_ATTR_BULLET_STYLE_ROMAN_UPPER|wxTEXT_ATTR_BULLET_STYLE_ROMAN_LOWER)));
}

/*!
 * The style manager
 */

wxIMPLEMENT_CLASS(wxRichTextStyleSheet, wxObject);

wxRichTextStyleSheet::~wxRichTextStyleSheet()
{
    DeleteStyles();

    if (m_nextSheet)
        m_nextSheet->m_previousSheet = m_previousSheet;

    if (m_previousSheet)
        m_previousSheet->m_nextSheet = m_nextSheet;

    m_previousSheet = nullptr;
    m_nextSheet = nullptr;
}

/// Add a definition to one of the style lists
bool wxRichTextStyleSheet::AddStyle(wxList& list, wxRichTextStyleDefinition* def)
{
    if (!list.Find(def))
        list.Append(def);
    return true;
}

/// Remove a style
bool wxRichTextStyleSheet::RemoveStyle(wxList& list, wxRichTextStyleDefinition* def, bool deleteStyle)
{
    wxList::compatibility_iterator node = list.Find(def);
    if (node)
    {
        wxRichTextStyleDefinition* nodeDef = (wxRichTextStyleDefinition*) node->GetData();
        list.Erase(node);
        if (deleteStyle)
            delete nodeDef;
        return true;
    }
    else
        return false;
}

/// Remove a style
bool wxRichTextStyleSheet::RemoveStyle(wxRichTextStyleDefinition* def, bool deleteStyle)
{
    if (RemoveParagraphStyle(def, deleteStyle))
        return true;
    if (RemoveCharacterStyle(def, deleteStyle))
        return true;
    if (RemoveListStyle(def, deleteStyle))
        return true;
    if (RemoveBoxStyle(def, deleteStyle))
        return true;
    return false;
}

/// Find a definition by name
wxRichTextStyleDefinition* wxRichTextStyleSheet::FindStyle(const wxList& list, const std::string& name, bool recurse) const
{
    for (wxList::compatibility_iterator node = list.GetFirst(); node; node = node->GetNext())
    {
        wxRichTextStyleDefinition* def = (wxRichTextStyleDefinition*) node->GetData();
        if (def->GetName() == name)
            return def;
    }

    if (m_nextSheet && recurse)
        return m_nextSheet->FindStyle(list, name, recurse);

    return nullptr;
}

/// Delete all styles
void wxRichTextStyleSheet::DeleteStyles()
{
    WX_CLEAR_LIST(wxList, m_characterStyleDefinitions);
    WX_CLEAR_LIST(wxList, m_paragraphStyleDefinitions);
    WX_CLEAR_LIST(wxList, m_listStyleDefinitions);
    WX_CLEAR_LIST(wxList, m_boxStyleDefinitions);
}

/// Insert into list of style sheets
bool wxRichTextStyleSheet::InsertSheet(wxRichTextStyleSheet* before)
{
    m_previousSheet = before->m_previousSheet;
    m_nextSheet = before;

    before->m_previousSheet = this;
    return true;
}

/// Append to list of style sheets
bool wxRichTextStyleSheet::AppendSheet(wxRichTextStyleSheet* after)
{
    wxRichTextStyleSheet* last = after;
    while (last && last->m_nextSheet)
    {
        last = last->m_nextSheet;
    }

    if (last)
    {
        m_previousSheet = last;
        last->m_nextSheet = this;

        return true;
    }
    else
        return false;
}

/// Unlink from the list of style sheets
void wxRichTextStyleSheet::Unlink()
{
    if (m_previousSheet)
        m_previousSheet->m_nextSheet = m_nextSheet;
    if (m_nextSheet)
        m_nextSheet->m_previousSheet = m_previousSheet;

    m_previousSheet = nullptr;
    m_nextSheet = nullptr;
}

/// Add a definition to the character style list
bool wxRichTextStyleSheet::AddCharacterStyle(wxRichTextCharacterStyleDefinition* def)
{
    def->GetStyle().SetCharacterStyleName(def->GetName());
    return AddStyle(m_characterStyleDefinitions, def);
}

/// Add a definition to the paragraph style list
bool wxRichTextStyleSheet::AddParagraphStyle(wxRichTextParagraphStyleDefinition* def)
{
    def->GetStyle().SetParagraphStyleName(def->GetName());
    return AddStyle(m_paragraphStyleDefinitions, def);
}

/// Add a definition to the list style list
bool wxRichTextStyleSheet::AddListStyle(wxRichTextListStyleDefinition* def)
{
    def->GetStyle().SetListStyleName(def->GetName());
    return AddStyle(m_listStyleDefinitions, def);
}

/// Add a definition to the box style list
bool wxRichTextStyleSheet::AddBoxStyle(wxRichTextBoxStyleDefinition* def)
{
    def->GetStyle().GetTextBoxAttr().SetBoxStyleName(def->GetName());
    return AddStyle(m_boxStyleDefinitions, def);
}

/// Add a definition to the appropriate style list
bool wxRichTextStyleSheet::AddStyle(wxRichTextStyleDefinition* def)
{
    wxRichTextListStyleDefinition* listDef = wxDynamicCast(def, wxRichTextListStyleDefinition);
    if (listDef)
        return AddListStyle(listDef);

    wxRichTextParagraphStyleDefinition* paraDef = wxDynamicCast(def, wxRichTextParagraphStyleDefinition);
    if (paraDef)
        return AddParagraphStyle(paraDef);

    wxRichTextCharacterStyleDefinition* charDef = wxDynamicCast(def, wxRichTextCharacterStyleDefinition);
    if (charDef)
        return AddCharacterStyle(charDef);

    wxRichTextBoxStyleDefinition* boxDef = wxDynamicCast(def, wxRichTextBoxStyleDefinition);
    if (boxDef)
        return AddBoxStyle(boxDef);

    return false;
}

/// Find any definition by name
wxRichTextStyleDefinition* wxRichTextStyleSheet::FindStyle(const std::string& name, bool recurse) const
{
    wxRichTextListStyleDefinition* listDef = FindListStyle(name, recurse);
    if (listDef)
        return listDef;

    wxRichTextParagraphStyleDefinition* paraDef = FindParagraphStyle(name, recurse);
    if (paraDef)
        return paraDef;

    wxRichTextCharacterStyleDefinition* charDef = FindCharacterStyle(name, recurse);
    if (charDef)
        return charDef;

    wxRichTextBoxStyleDefinition* boxDef = FindBoxStyle(name, recurse);
    if (boxDef)
        return boxDef;

    return nullptr;
}

/// Copy
void wxRichTextStyleSheet::Copy(const wxRichTextStyleSheet& sheet)
{
    DeleteStyles();

    wxList::compatibility_iterator node;

    for (node = sheet.m_characterStyleDefinitions.GetFirst(); node; node = node->GetNext())
    {
        wxRichTextCharacterStyleDefinition* def = (wxRichTextCharacterStyleDefinition*) node->GetData();
        AddCharacterStyle(new wxRichTextCharacterStyleDefinition(*def));
    }

    for (node = sheet.m_paragraphStyleDefinitions.GetFirst(); node; node = node->GetNext())
    {
        wxRichTextParagraphStyleDefinition* def = (wxRichTextParagraphStyleDefinition*) node->GetData();
        AddParagraphStyle(new wxRichTextParagraphStyleDefinition(*def));
    }

    for (node = sheet.m_listStyleDefinitions.GetFirst(); node; node = node->GetNext())
    {
        wxRichTextListStyleDefinition* def = (wxRichTextListStyleDefinition*) node->GetData();
        AddListStyle(new wxRichTextListStyleDefinition(*def));
    }

    for (node = sheet.m_boxStyleDefinitions.GetFirst(); node; node = node->GetNext())
    {
        wxRichTextBoxStyleDefinition* def = (wxRichTextBoxStyleDefinition*) node->GetData();
        AddBoxStyle(new wxRichTextBoxStyleDefinition(*def));
    }

    SetName(sheet.GetName());
    SetDescription(sheet.GetDescription());
    m_properties = sheet.m_properties;
}

/// Equality
bool wxRichTextStyleSheet::operator==([[maybe_unused]] const wxRichTextStyleSheet& sheet) const
{
    // TODO
    return false;
}


#if wxUSE_HTML

// Functions for dealing with clashing names for different kinds of style.
// Returns "P", "C", "L" or "B" (paragraph, character, list or box) for
// style name | type.
static wxString wxGetRichTextStyleType(const wxString& style)
{
    return style.AfterLast(wxT('|'));
}

static wxString wxGetRichTextStyle(const wxString& style)
{
    return style.BeforeLast(wxT('|'));
}


/*!
 * wxRichTextStyleListBox: a listbox to display styles.
 */

wxIMPLEMENT_CLASS(wxRichTextStyleListBox, wxHtmlListBox);

wxBEGIN_EVENT_TABLE(wxRichTextStyleListBox, wxHtmlListBox)
    EVT_LEFT_DOWN(wxRichTextStyleListBox::OnLeftDown)
    EVT_LEFT_DCLICK(wxRichTextStyleListBox::OnLeftDoubleClick)
    EVT_IDLE(wxRichTextStyleListBox::OnIdle)
wxEND_EVENT_TABLE()

wxRichTextStyleListBox::wxRichTextStyleListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, unsigned int style)
{
    Create(parent, id, pos, size, style);
}

bool wxRichTextStyleListBox::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, unsigned int style)
{
    return wxHtmlListBox::Create(parent, id, pos, size, style);
}

/// Returns the HTML for this item
std::string wxRichTextStyleListBox::OnGetItem(size_t n) const
{
    if (!GetStyleSheet())
        return {};

    wxRichTextStyleDefinition* def = GetStyle(n);
    if (def)
        return CreateHTML(def);

    return {};
}

// Get style for index
wxRichTextStyleDefinition* wxRichTextStyleListBox::GetStyle(size_t i) const
{
    if (!GetStyleSheet())
        return nullptr;

    if (i >= m_styleNames.GetCount() /* || i < 0 */ )
        return nullptr;

    wxString styleType = wxGetRichTextStyleType(m_styleNames[i]);
    wxString style = wxGetRichTextStyle(m_styleNames[i]);
    if (styleType == "P")
        return GetStyleSheet()->FindParagraphStyle(style);
    else if (styleType == "C")
        return GetStyleSheet()->FindCharacterStyle(style);
    else if (styleType == "L")
        return GetStyleSheet()->FindListStyle(style);
    else if (styleType == "B")
        return GetStyleSheet()->FindBoxStyle(style);
    else
        return GetStyleSheet()->FindStyle(style);
}

/// Updates the list
void wxRichTextStyleListBox::UpdateStyles()
{
    if (GetStyleSheet())
    {
        int oldSel = GetSelection();

        SetSelection(wxNOT_FOUND);

        m_styleNames.Clear();

        size_t i;
        if (GetStyleType() == wxRichTextStyleType::All || GetStyleType() == wxRichTextStyleType::Paragraph)
        {
            for (i = 0; i < GetStyleSheet()->GetParagraphStyleCount(); i++)
                m_styleNames.Add(GetStyleSheet()->GetParagraphStyle(i)->GetName() + "|P");
        }
        if (GetStyleType() == wxRichTextStyleType::All || GetStyleType() == wxRichTextStyleType::Character)
        {
            for (i = 0; i < GetStyleSheet()->GetCharacterStyleCount(); i++)
                m_styleNames.Add(GetStyleSheet()->GetCharacterStyle(i)->GetName() + "|C");
        }
        if (GetStyleType() == wxRichTextStyleType::All || GetStyleType() == wxRichTextStyleType::List)
        {
            for (i = 0; i < GetStyleSheet()->GetListStyleCount(); i++)
                m_styleNames.Add(GetStyleSheet()->GetListStyle(i)->GetName() + "|L");
        }
        if (GetStyleType() == wxRichTextStyleType::All || GetStyleType() == wxRichTextStyleType::Box)
        {
            for (i = 0; i < GetStyleSheet()->GetBoxStyleCount(); i++)
                m_styleNames.Add(GetStyleSheet()->GetBoxStyle(i)->GetName() + "|B");
        }

        m_styleNames.Sort();
        SetItemCount(m_styleNames.GetCount());

        Refresh();

        int newSel = -1;
        if (oldSel >= 0 && oldSel < (int) GetItemCount())
            newSel = oldSel;
        else if (GetItemCount() > 0)
            newSel = 0;

        if (newSel >= 0)
        {
            SetSelection(newSel);
            SendSelectedEvent();
        }
    }
    else
    {
        m_styleNames.Clear();
        SetSelection(wxNOT_FOUND);
        SetItemCount(0);
        Refresh();
    }
}

// Get index for style name
int wxRichTextStyleListBox::GetIndexForStyle(const std::string& name) const
{
    std::string s{name};
    if (GetStyleType() == wxRichTextStyleType::Paragraph)
        s += "|P";
    else if (GetStyleType() == wxRichTextStyleType::Character)
        s += "|C";
    else if (GetStyleType() == wxRichTextStyleType::List)
        s += "|L";
    else if (GetStyleType() == wxRichTextStyleType::Box)
        s += "|B";
    else
    {
        if (m_styleNames.Index(s + "|P") != wxNOT_FOUND)
            s += "|P";
        else if (m_styleNames.Index(s + "|C") != wxNOT_FOUND)
            s += "|C";
        else if (m_styleNames.Index(s + "|L") != wxNOT_FOUND)
            s += "|L";
        else if (m_styleNames.Index(s + "|B") != wxNOT_FOUND)
            s += "|B";
    }
    return m_styleNames.Index(s);
}

/// Set selection for string
int wxRichTextStyleListBox::SetStyleSelection(const std::string& name)
{
    int i = GetIndexForStyle(name);
    if (i > -1)
    {
        SetSelection(i);
        if (!IsVisible(i))
            ScrollToRow(i);
    }
    return i;
}

// Convert a colour to a 6-digit hex string
static wxString ColourToHexString(const wxColour& col)
{
    wxString hex;

    hex += wxDecToHex(col.Red());
    hex += wxDecToHex(col.Green());
    hex += wxDecToHex(col.Blue());

    return hex;
}

/// Creates a suitable HTML fragment for a definition
std::string wxRichTextStyleListBox::CreateHTML(wxRichTextStyleDefinition* def) const
{
    // TODO: indicate list format for list style types

    wxString str;

    bool isCentred = false;

    wxRichTextAttr attr(def->GetStyleMergedWithBase(GetStyleSheet()));

    if (attr.HasAlignment() && attr.GetAlignment() == wxTextAttrAlignment::Centre)
        isCentred = true;

    str << "<html><head></head>";
    str << "<body";
    if (attr.GetBackgroundColour().IsOk())
        str << " bgcolor=\"#" << ColourToHexString(attr.GetBackgroundColour()) << "\"";
    str << ">";

    if (isCentred)
        str << "<center>";

    str << "<table";
    if (attr.GetBackgroundColour().IsOk())
        str << " bgcolor=\"#" << ColourToHexString(attr.GetBackgroundColour()) << "\"";

    str << "><tr>";

    if (attr.GetLeftIndent() > 0)
    {
        wxClientDC dc(const_cast<wxRichTextStyleListBox*>(this));

        str << "<td width=" << std::min(50, (ConvertTenthsMMToPixels(dc, attr.GetLeftIndent())/2)) << "></td>";
    }

    if (isCentred)
        str << "<td nowrap align=\"center\">";
    else
        str << "<td nowrap>";

#ifdef __WXMSW__
    int size = 2;
#else
    int size = 3;
#endif

    // Guess a standard font size
    int stdFontSize = 0;

    // First see if we have a default/normal style to base the size on
    wxString normalTranslated(_("normal"));
    wxString defaultTranslated(_("default"));
    size_t i;
    for (i = 0; i < GetStyleSheet()->GetParagraphStyleCount(); i++)
    {
        wxRichTextStyleDefinition* d = GetStyleSheet()->GetParagraphStyle(i);
        std::string name = wx::utils::ToLowerCopy(d->GetName());
        if (name.find("normal") != std::string::npos || name.find(normalTranslated) != std::string::npos ||
            name.find("default") != std::string::npos || name.find(defaultTranslated) != std::string::npos)
        {
            wxRichTextAttr attr2(d->GetStyleMergedWithBase(GetStyleSheet()));
            if (attr2.HasFontPointSize())
            {
                stdFontSize = attr2.GetFontSize();
                break;
            }
        }
    }

    if (stdFontSize == 0)
    {
        // Look at sizes up to 20 points, and see which is the most common
        std::vector<int> sizes;
        size_t maxSize = 20;
        for (i = 0; i <= maxSize; i++)
            sizes.push_back(0);
        for (i = 0; i < m_styleNames.GetCount(); i++)
        {
            wxRichTextStyleDefinition* d = GetStyle(i);
            if (d)
            {
                wxRichTextAttr attr2(d->GetStyleMergedWithBase(GetStyleSheet()));
                if (attr2.HasFontPointSize())
                {
                    if (attr2.GetFontSize() <= (int) maxSize)
                        sizes[attr2.GetFontSize()] ++;
                }
            }
        }
        int mostCommonSize = 0;
        for (i = 0; i <= maxSize; i++)
        {
            if (sizes[i] > mostCommonSize)
                mostCommonSize = i;
        }
        if (mostCommonSize > 0)
            stdFontSize = mostCommonSize;
    }

    if (stdFontSize == 0)
        stdFontSize = 12;

    int thisFontSize = attr.HasFontPointSize() ? attr.GetFontSize() : stdFontSize;

    if (thisFontSize < stdFontSize)
        size --;
    else if (thisFontSize > stdFontSize)
        size ++;

    str += "<font";

    str << " size=" << size;

    if (!attr.GetFontFaceName().IsEmpty())
        str << " face=\"" << attr.GetFontFaceName() << "\"";

    if (attr.GetTextColour().IsOk() && attr.GetTextColour() != attr.GetBackgroundColour() && !(!attr.HasBackgroundColour() && attr.GetTextColour() == *wxWHITE))
        str << " color=\"#" << ColourToHexString(attr.GetTextColour()) << "\"";

    if (attr.GetBackgroundColour().IsOk())
        str << " bgcolor=\"#" << ColourToHexString(attr.GetBackgroundColour()) << "\"";

    str << ">";

    bool hasBold = false;
    bool hasItalic = false;
    bool hasUnderline = false;

    if (attr.GetFontWeight() == wxFONTWEIGHT_BOLD)
        hasBold = true;
    if (attr.GetFontStyle() == wxFontStyle::Italic)
        hasItalic = true;
    if (attr.GetFontUnderlined())
        hasUnderline = true;

    if (hasBold)
        str << "<b>";
    if (hasItalic)
        str << "<i>";
    if (hasUnderline)
        str << "<u>";

    wxString name(def->GetName());
    if (attr.HasTextEffects() && (attr.GetTextEffects() & (wxTEXT_ATTR_EFFECT_CAPITALS|wxTEXT_ATTR_EFFECT_SMALL_CAPITALS)))
        name = name.Upper();

    str += name;

    if (hasUnderline)
        str << "</u>";
    if (hasItalic)
        str << "</i>";
    if (hasBold)
        str << "</b>";

    if (isCentred)
        str << "</centre>";

    str << "</font>";

    str << "</td></tr></table>";

    if (isCentred)
        str << "</center>";

    str << "</body></html>";
    return str;
}

// Convert units in tends of a millimetre to device units
int wxRichTextStyleListBox::ConvertTenthsMMToPixels(wxDC& dc, int units) const
{
    int ppi = dc.GetPPI().x;

    // There are ppi pixels in 254.1 "1/10 mm"

    double pixels = ((double) units * (double)ppi) / 254.1;

    return (int) pixels;
}

void wxRichTextStyleListBox::OnLeftDown(wxMouseEvent& event)
{
    wxVListBox::OnLeftDown(event);

    int item = VirtualHitTest(event.GetPosition().y);
    if (item != wxNOT_FOUND && GetApplyOnSelection())
        ApplyStyle(item);
}

void wxRichTextStyleListBox::OnLeftDoubleClick(wxMouseEvent& event)
{
    wxVListBox::OnLeftDown(event);

    int item = VirtualHitTest(event.GetPosition().y);
    if (item != wxNOT_FOUND && !GetApplyOnSelection())
        ApplyStyle(item);
}

/// Helper for listbox and combo control
std::string wxRichTextStyleListBox::GetStyleToShowInIdleTime(wxRichTextCtrl* ctrl, wxRichTextStyleType styleType)
{
    int adjustedCaretPos = ctrl->GetAdjustedCaretPosition(ctrl->GetCaretPosition());

    wxString styleName;

    wxRichTextAttr attr;
    ctrl->GetStyle(adjustedCaretPos, attr);

    // Take into account current default style just chosen by user
    if (ctrl->IsDefaultStyleShowing())
    {
        wxRichTextApplyStyle(attr, ctrl->GetDefaultStyleEx());

        if ((styleType == wxRichTextStyleType::All || styleType == wxRichTextStyleType::Character) &&
                          !attr.GetCharacterStyleName().IsEmpty())
            styleName = attr.GetCharacterStyleName();
        else if ((styleType == wxRichTextStyleType::All || styleType == wxRichTextStyleType::Paragraph) &&
                          !attr.GetParagraphStyleName().IsEmpty())
            styleName = attr.GetParagraphStyleName();
        else if ((styleType == wxRichTextStyleType::All || styleType == wxRichTextStyleType::List) &&
                          !attr.GetListStyleName().IsEmpty())
            styleName = attr.GetListStyleName();
        // TODO: when we have a concept of focused object (text box), we'll
        // use the paragraph style name of the focused object as the frame style name.
#if 0
        else if ((styleType == wxRichTextStyleType::All || styleType == wxRichTextStyleType::Box) &&
                          !attr.GetBoxStyleName().IsEmpty())
            styleName = attr.GetBoxStyleName();
#endif
    }
    else if ((styleType == wxRichTextStyleType::All || styleType == wxRichTextStyleType::Character) &&
             !attr.GetCharacterStyleName().IsEmpty())
    {
        styleName = attr.GetCharacterStyleName();
    }
    else if ((styleType == wxRichTextStyleType::All || styleType == wxRichTextStyleType::Paragraph) &&
             !attr.GetParagraphStyleName().IsEmpty())
    {
        styleName = attr.GetParagraphStyleName();
    }
    else if ((styleType == wxRichTextStyleType::All || styleType == wxRichTextStyleType::List) &&
             !attr.GetListStyleName().IsEmpty())
    {
        styleName = attr.GetListStyleName();
    }

    return styleName;
}

/// Auto-select from style under caret in idle time
void wxRichTextStyleListBox::OnIdle(wxIdleEvent& event)
{
    if (CanAutoSetSelection() && GetRichTextCtrl() && IsShownOnScreen() && wxWindow::FindFocus() != this)
    {
        wxString styleName = GetStyleToShowInIdleTime(GetRichTextCtrl(), GetStyleType());

        int sel = GetSelection();
        if (!styleName.IsEmpty())
        {
            // Don't do the selection if it's already set
            if (sel == GetIndexForStyle(styleName))
                return;

            SetStyleSelection(styleName);
        }
        else if (sel != -1)
            SetSelection(-1);
    }
    event.Skip();
}

/// Do selection
void wxRichTextStyleListBox::ApplyStyle(int item)
{
    if ( item != wxNOT_FOUND )
    {
        wxRichTextStyleDefinition* def = GetStyle(item);
        if (def && GetRichTextCtrl())
        {
            GetRichTextCtrl()->ApplyStyle(def);
            GetRichTextCtrl()->SetFocus();
        }
    }
}

/*!
 * wxRichTextStyleListCtrl class: manages a listbox and a choice control to
 * switch shown style types
 */

wxIMPLEMENT_CLASS(wxRichTextStyleListCtrl, wxControl);

wxBEGIN_EVENT_TABLE(wxRichTextStyleListCtrl, wxControl)
    EVT_CHOICE(wxID_ANY, wxRichTextStyleListCtrl::OnChooseType)
    EVT_SIZE(wxRichTextStyleListCtrl::OnSize)
wxEND_EVENT_TABLE()

wxRichTextStyleListCtrl::wxRichTextStyleListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, unsigned int style)
{
    Create(parent, id, pos, size, style);
}

bool wxRichTextStyleListCtrl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, unsigned int style)
{
    if ((style & wxBORDER_MASK) == wxBORDER_DEFAULT)
        style |= wxBORDER_THEME;

    wxControl::Create(parent, id, pos, size, style);

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    if (size != wxDefaultSize)
        SetInitialSize(size);

    bool showSelector = ((style & wxRICHTEXTSTYLELIST_HIDE_TYPE_SELECTOR) == 0);

    wxBorder listBoxStyle;
    if (showSelector)
        listBoxStyle = wxBORDER_THEME;
    else
        listBoxStyle = wxBORDER_NONE;

    m_styleListBox = new wxRichTextStyleListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, listBoxStyle);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);

    if (showSelector)
    {
        std::vector<std::string> choices;
        choices.push_back("All styles");
        choices.push_back("Paragraph styles");
        choices.push_back("Character styles");
        choices.push_back("List styles");
        choices.push_back("Box styles");

        m_styleChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);

        boxSizer->Add(m_styleListBox, 1, wxALL|wxEXPAND, 5);
        boxSizer->Add(m_styleChoice, 0, wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxBOTTOM | wxEXPAND, 5);
    }
    else
    {
        boxSizer->Add(m_styleListBox, 1, wxALL|wxEXPAND, 0);
    }

    SetSizer(boxSizer);
    Layout();

    m_dontUpdate = true;

    if (m_styleChoice)
    {
        int i = StyleTypeToIndex(m_styleListBox->GetStyleType());
        m_styleChoice->SetSelection(i);
    }

    m_dontUpdate = false;

    return true;
}

/// React to style type choice
void wxRichTextStyleListCtrl::OnChooseType(wxCommandEvent& event)
{
    if (event.GetEventObject() != m_styleChoice)
        event.Skip();
    else
    {
        if (m_dontUpdate)
            return;

        wxRichTextStyleListBox::wxRichTextStyleType styleType = StyleIndexToType(event.GetSelection());
        m_styleListBox->SetSelection(-1);
        m_styleListBox->SetStyleType(styleType);
    }
}

/// Lay out the controls
void wxRichTextStyleListCtrl::OnSize([[maybe_unused]] wxSizeEvent& event)
{
    if (GetAutoLayout())
        Layout();
}

/// Get the choice index for style type
int wxRichTextStyleListCtrl::StyleTypeToIndex(wxRichTextStyleListBox::wxRichTextStyleType styleType)
{
    if (styleType == wxRichTextStyleListBox::wxRichTextStyleType::All)
    {
        return 0;
    }
    else if (styleType == wxRichTextStyleListBox::wxRichTextStyleType::Paragraph)
    {
        return 1;
    }
    else if (styleType == wxRichTextStyleListBox::wxRichTextStyleType::Character)
    {
        return 2;
    }
    else if (styleType == wxRichTextStyleListBox::wxRichTextStyleType::List)
    {
        return 3;
    }
    else if (styleType == wxRichTextStyleListBox::wxRichTextStyleType::Box)
    {
        return 4;
    }
    return 0;
}

/// Get the style type for choice index
wxRichTextStyleListBox::wxRichTextStyleType wxRichTextStyleListCtrl::StyleIndexToType(int i)
{
    if (i == 1)
        return wxRichTextStyleListBox::wxRichTextStyleType::Paragraph;
    else if (i == 2)
        return wxRichTextStyleListBox::wxRichTextStyleType::Character;
    else if (i == 3)
        return wxRichTextStyleListBox::wxRichTextStyleType::List;
    else if (i == 4)
        return wxRichTextStyleListBox::wxRichTextStyleType::Box;

    return wxRichTextStyleListBox::wxRichTextStyleType::All;
}

/// Associates the control with a style manager
void wxRichTextStyleListCtrl::SetStyleSheet(wxRichTextStyleSheet* styleSheet)
{
    if (m_styleListBox)
        m_styleListBox->SetStyleSheet(styleSheet);
}

wxRichTextStyleSheet* wxRichTextStyleListCtrl::GetStyleSheet() const
{
    if (m_styleListBox)
        return m_styleListBox->GetStyleSheet();
    else
        return nullptr;
}

/// Associates the control with a wxRichTextCtrl
void wxRichTextStyleListCtrl::SetRichTextCtrl(wxRichTextCtrl* ctrl)
{
    if (m_styleListBox)
        m_styleListBox->SetRichTextCtrl(ctrl);
}

wxRichTextCtrl* wxRichTextStyleListCtrl::GetRichTextCtrl() const
{
    if (m_styleListBox)
        return m_styleListBox->GetRichTextCtrl();
    else
        return nullptr;
}

/// Set/get the style type to display
void wxRichTextStyleListCtrl::SetStyleType(wxRichTextStyleListBox::wxRichTextStyleType styleType)
{
    if ( !m_styleListBox )
        return;

    m_styleListBox->SetStyleType(styleType);

    m_dontUpdate = true;

    if (m_styleChoice)
    {
        int i = StyleTypeToIndex(m_styleListBox->GetStyleType());
        m_styleChoice->SetSelection(i);
    }

    m_dontUpdate = false;
}

wxRichTextStyleListBox::wxRichTextStyleType wxRichTextStyleListCtrl::GetStyleType() const
{
    if (m_styleListBox)
        return m_styleListBox->GetStyleType();
    else
        return wxRichTextStyleListBox::wxRichTextStyleType::All;
}

/// Updates the style list box
void wxRichTextStyleListCtrl::UpdateStyles()
{
    if (m_styleListBox)
        m_styleListBox->UpdateStyles();
}

#if wxUSE_COMBOCTRL

/*!
 * Style drop-down for a wxComboCtrl
 */


wxBEGIN_EVENT_TABLE(wxRichTextStyleComboPopup, wxRichTextStyleListBox)
    EVT_MOTION(wxRichTextStyleComboPopup::OnMouseMove)
    EVT_LEFT_DOWN(wxRichTextStyleComboPopup::OnMouseClick)
wxEND_EVENT_TABLE()

bool wxRichTextStyleComboPopup::Create( wxWindow* parent )
{
    int borderStyle = GetDefaultBorder();
    if (borderStyle == wxBORDER_SUNKEN || borderStyle == wxBORDER_NONE)
        borderStyle = wxBORDER_THEME;

    return wxRichTextStyleListBox::Create(parent, wxID_ANY,
                                  wxPoint(0,0), wxDefaultSize,
                                  borderStyle);
}

void wxRichTextStyleComboPopup::SetStringValue( const std::string& s )
{
    m_value = SetStyleSelection(s);
}

std::string wxRichTextStyleComboPopup::GetStringValue() const
{
    int sel = m_value;
    if (sel > -1)
    {
        wxRichTextStyleDefinition* def = GetStyle(sel);
        if (def)
            return def->GetName();
    }
    return "";
}

//
// Popup event handlers
//

// Mouse hot-tracking
void wxRichTextStyleComboPopup::OnMouseMove(wxMouseEvent& event)
{
    // Move selection to cursor if it is inside the popup

    int itemHere = wxRichTextStyleListBox::VirtualHitTest(event.GetPosition().y);
    if ( itemHere >= 0 )
    {
        wxRichTextStyleListBox::SetSelection(itemHere);
        m_itemHere = itemHere;
    }
    event.Skip();
}

// On mouse left, set the value and close the popup
void wxRichTextStyleComboPopup::OnMouseClick([[maybe_unused]] wxMouseEvent& event)
{
    if (m_itemHere >= 0)
        m_value = m_itemHere;

    // Ordering is important, so we don't dismiss this popup accidentally
    // by setting the focus elsewhere e.g. in ApplyStyle
    Dismiss();

    if (m_itemHere >= 0)
        wxRichTextStyleListBox::ApplyStyle(m_itemHere);
}

/*!
 * wxRichTextStyleComboCtrl
 * A combo for applying styles.
 */

wxIMPLEMENT_CLASS(wxRichTextStyleComboCtrl, wxComboCtrl);

wxBEGIN_EVENT_TABLE(wxRichTextStyleComboCtrl, wxComboCtrl)
    EVT_IDLE(wxRichTextStyleComboCtrl::OnIdle)
wxEND_EVENT_TABLE()

bool wxRichTextStyleComboCtrl::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, unsigned int style)
{
    if (!wxComboCtrl::Create(parent, id, "", pos, size, style))
        return false;

    SetPopupMaxHeight(400);

    m_stylePopup = new wxRichTextStyleComboPopup;

    SetPopupControl(m_stylePopup);

    return true;
}

/// Auto-select from style under caret in idle time

// TODO: must be able to show italic, bold, combinations
// in style box. Do we have a concept of automatic, temporary
// styles that are added whenever we wish to show a style
// that doesn't exist already? E.g. "Bold, Italic, Underline".
// Word seems to generate these things on the fly.
// If there's a named style already, it uses e.g. Heading1 + Bold, Italic
// If you unembolden text in a style that has bold, it uses the
// term "Not bold".
// TODO: order styles alphabetically. This means indexes can change,
// so need a different way to specify selections, i.e. by name.

void wxRichTextStyleComboCtrl::OnIdle(wxIdleEvent& event)
{
    event.Skip();

    if ( !m_stylePopup )
        return;

    wxRichTextCtrl * const richtext = GetRichTextCtrl();
    if ( !richtext )
        return;

    if ( !IsPopupShown() && IsShownOnScreen() && wxWindow::FindFocus() != this )
    {
        wxString styleName =
            wxRichTextStyleListBox::GetStyleToShowInIdleTime(richtext, m_stylePopup->GetStyleType());

        wxString currentValue = GetValue();
        if (!styleName.IsEmpty())
        {
            // Don't do the selection if it's already set
            if (currentValue == styleName)
                return;

            SetValue(styleName);
        }
        else if (!currentValue.IsEmpty())
            SetValue("");
    }
}

#endif
    // wxUSE_COMBOCTRL

#endif
    // wxUSE_HTML

#endif
    // wxUSE_RICHTEXT
