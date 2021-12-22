/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtexthtml.cpp
// Purpose:     HTML I/O for wxRichTextCtrl
// Author:      Julian Smart
// Modified by:
// Created:     2005-09-30
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_RICHTEXT

#include "wx/richtext/richtexthtml.h"
#include "wx/richtext/richtextstyles.h"

#ifndef WX_PRECOMP
#endif

#include "wx/filename.h"

#if wxUSE_FILESYSTEM
#include "wx/filesys.h"
#include "wx/fs_mem.h"
#endif

import WX.Cmn.TextStream;
import WX.Cmn.WFStream;

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextHTMLHandler, wxRichTextFileHandler);

int wxRichTextHTMLHandler::sm_fileCounter = 1;

wxRichTextHTMLHandler::wxRichTextHTMLHandler(const std::string& name, const std::string& ext, int type)
    : wxRichTextFileHandler(name, ext, type) 
{
    m_fontSizeMapping.push_back(8);
    m_fontSizeMapping.push_back(10);
    m_fontSizeMapping.push_back(13);
    m_fontSizeMapping.push_back(17);
    m_fontSizeMapping.push_back(22);
    m_fontSizeMapping.push_back(30);
    m_fontSizeMapping.push_back(100);
}

/// Can we handle this filename (if using files)? By default, checks the extension.
bool wxRichTextHTMLHandler::CanHandle(const std::string& filename) const
{
    wxString path, file, ext;
    wxFileName::SplitPath(filename, & path, & file, & ext);

    return (ext.Lower() == "html") || ext.Lower() == wxT("htm");
}


#if wxUSE_STREAMS
bool wxRichTextHTMLHandler::DoLoadFile([[maybe_unused]] wxRichTextBuffer *buffer, [[maybe_unused]] wxInputStream& stream)
{
    return false;
}

/*
 * We need to output only _changes_ in character formatting.
 */

bool wxRichTextHTMLHandler::DoSaveFile(wxRichTextBuffer *buffer, wxOutputStream& stream)
{
    m_buffer = buffer;

    ClearTemporaryImageLocations();

    wxRichTextDrawingContext context(buffer);
    buffer->Defragment(context);

    wxCSConv* customEncoding = nullptr;
    wxMBConv* conv = nullptr;
    if (!GetEncoding().IsEmpty())
    {
        customEncoding = new wxCSConv(GetEncoding());
        if (!customEncoding->IsOk())
        {
            wxDELETE(customEncoding);
        }
    }
    if (customEncoding)
        conv = customEncoding;
    else
        conv = & wxConvUTF8;

    {
        wxTextOutputStream str(stream, wxEOL::Native, *conv);

        wxRichTextAttr currentParaStyle = buffer->GetAttributes();
        wxRichTextAttr currentCharStyle = buffer->GetAttributes();

        if ((GetFlags() & wxRICHTEXT_HANDLER_NO_HEADER_FOOTER) == 0)
            str << "<html><head></head><body>\n";

        OutputFont(currentParaStyle, str);

        m_font = false;
        m_inTable = false;

        m_indents.clear();
        m_listTypes.clear();

        wxRichTextObjectList::compatibility_iterator node = buffer->GetChildren().GetFirst();
        while (node)
        {
            wxRichTextParagraph* para = wxDynamicCast(node->GetData(), wxRichTextParagraph);
            wxASSERT (para != nullptr);

            if (para)
            {
                wxRichTextAttr paraStyle(para->GetCombinedAttributes());

                BeginParagraphFormatting(currentParaStyle, paraStyle, str);

                wxRichTextObjectList::compatibility_iterator node2 = para->GetChildren().GetFirst();
                while (node2)
                {
                    wxRichTextObject* obj = node2->GetData();
                    wxRichTextPlainText* textObj = wxDynamicCast(obj, wxRichTextPlainText);
                    if (textObj && !textObj->IsEmpty())
                    {
                        wxRichTextAttr charStyle(para->GetCombinedAttributes(obj->GetAttributes()));
                        BeginCharacterFormatting(currentCharStyle, charStyle, paraStyle, str);

                        std::string text = textObj->GetText();

                        if (charStyle.HasTextEffects() && (charStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_CAPITALS))
                            wx::utils::ToUpper(text);

                        std::string toReplace{wxRichTextLineBreakChar};
                        wx::utils::ReplaceAll(text, toReplace, "<br>");

                        str << text;

                        EndCharacterFormatting(currentCharStyle, charStyle, paraStyle, str);
                    }

                    wxRichTextImage* image = wxDynamicCast(obj, wxRichTextImage);
                    if( image && (!image->IsEmpty() || image->GetImageBlock().GetData()))
                        WriteImage( image, stream );

                    node2 = node2->GetNext();
                }

                EndParagraphFormatting(currentParaStyle, paraStyle, str);

                str << "\n";
            }
            node = node->GetNext();
        }

        CloseLists(-1, str);

        if (currentParaStyle.HasFont())
            str << "</font>";

        if ((GetFlags() & wxRICHTEXT_HANDLER_NO_HEADER_FOOTER) == 0)
            str << "</body></html>";

        str << "\n";
    }

    if (customEncoding)
        delete customEncoding;

    m_buffer = nullptr;

    return true;
}

void wxRichTextHTMLHandler::BeginCharacterFormatting(const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, [[maybe_unused]] const wxRichTextAttr& paraStyle, wxTextOutputStream& str)
{
    std::string style;

    // Is there any change in the font properties of the item?
    if (thisStyle.GetFontFaceName() != currentStyle.GetFontFaceName())
    {
        std::string faceName(thisStyle.GetFontFaceName());
        style += fmt::format(" face=\"{:s}\"", faceName.c_str());
    }
    if (thisStyle.GetFontSize() != currentStyle.GetFontSize())
        style += fmt::format(" size=\"{:ld}\"", PtToSize(thisStyle.GetFontSize()));

    bool bTextColourChanged = (thisStyle.GetTextColour() != currentStyle.GetTextColour());
    bool bBackgroundColourChanged = (thisStyle.GetBackgroundColour() != currentStyle.GetBackgroundColour());
    if (bTextColourChanged || bBackgroundColourChanged)
    {
        style += " style=\"";

        if (bTextColourChanged)
        {
            std::string color(thisStyle.GetTextColour().GetAsString(wxC2S_HTML_SYNTAX));
            style += fmt::format("color: {:s}", color.c_str());
        }
        if (bTextColourChanged && bBackgroundColourChanged)
            style += ";";
        if (bBackgroundColourChanged)
        {
            std::string color(thisStyle.GetBackgroundColour().GetAsString(wxC2S_HTML_SYNTAX));
            style += fmt::format("background-color: {:s}", color.c_str());
        }

        style += "\"";
    }

    if (style.size())
    {
        str << fmt::format("<font {:s} >", style.c_str());
        m_font = true;
    }

    if (thisStyle.GetFontWeight() == wxFONTWEIGHT_BOLD)
        str << "<b>";
    if (thisStyle.GetFontStyle() == wxFontStyle::Italic)
        str << "<i>";
    if (thisStyle.GetFontUnderlined())
        str << "<u>";

    if (thisStyle.HasURL())
        str << "<a href=\"" << thisStyle.GetURL() << "\">";

    if (thisStyle.HasTextEffects())
    {
        if (thisStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_STRIKETHROUGH)
            str << "<del>";
        if (thisStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_SUPERSCRIPT)
            str << "<sup>";
        if (thisStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_SUBSCRIPT)
            str << "<sub>";
    }
}

void wxRichTextHTMLHandler::EndCharacterFormatting([[maybe_unused]] const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, [[maybe_unused]] const wxRichTextAttr& paraStyle, wxTextOutputStream& stream)
{
    if (thisStyle.HasURL())
        stream << "</a>";

    if (thisStyle.GetFontUnderlined())
        stream << "</u>";
    if (thisStyle.GetFontStyle() == wxFontStyle::Italic)
        stream << "</i>";
    if (thisStyle.GetFontWeight() == wxFONTWEIGHT_BOLD)
        stream << "</b>";

    if (thisStyle.HasTextEffects())
    {
        if (thisStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_STRIKETHROUGH)
            stream << "</del>";
        if (thisStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_SUPERSCRIPT)
            stream << "</sup>";
        if (thisStyle.GetTextEffects() & wxTEXT_ATTR_EFFECT_SUBSCRIPT)
            stream << "</sub>";
    }

    if (m_font)
    {
        m_font = false;
        stream << "</font>";
    }
}

/// Begin paragraph formatting
void wxRichTextHTMLHandler::BeginParagraphFormatting([[maybe_unused]] const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, wxTextOutputStream& str)
{
    if (thisStyle.HasPageBreak())
    {
        str << "<div style=\"page-break-after:always\"></div>\n";
    }

    if (thisStyle.HasLeftIndent() && thisStyle.GetLeftIndent() != 0)
    {
        if (thisStyle.HasBulletStyle())
        {
            int indent = thisStyle.GetLeftIndent();

            // Close levels high than this
            CloseLists(indent, str);

            if (m_indents.size() > 0 && indent == m_indents.back())
            {
                // Same level, no need to start a new list
            }
            else if (m_indents.size() == 0 || indent > m_indents.back())
            {
                m_indents.push_back(indent);

                std::string tag;
                int listType = TypeOfList(thisStyle, tag);
                m_listTypes.push_back(listType);

                // wxHTML needs an extra <p> before a list when using <p> ... </p> in previous paragraphs.
                // TODO: pass a flag that indicates we're using wxHTML.
                str << "<p>\n";

                str << tag;
            }

            str << "<li> ";
        }
        else
        {
            CloseLists(-1, str);

            std::string align = GetAlignment(thisStyle);
            str << fmt::format("<p align=\"{:s}\"", align.c_str());

            std::string styleStr;

            if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) && thisStyle.HasParagraphSpacingBefore())
            {
                double spacingBeforeMM = thisStyle.GetParagraphSpacingBefore() / 10.0;

                styleStr += fmt::format("margin-top: {:.2f}mm; ", spacingBeforeMM);
            }
            if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) && thisStyle.HasParagraphSpacingAfter())
            {
                double spacingAfterMM = thisStyle.GetParagraphSpacingAfter() / 10.0;

                styleStr += fmt::format("margin-bottom: {:.2f}mm; ", spacingAfterMM);
            }

            double indentLeftMM = (thisStyle.GetLeftIndent() + thisStyle.GetLeftSubIndent()) / 10.0;
            if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) && (indentLeftMM > 0.0))
            {
                styleStr += fmt::format("margin-left: {:.2f}mm; ", indentLeftMM);
            }
            double indentRightMM = thisStyle.GetRightIndent() / 10.0;
            if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) && thisStyle.HasRightIndent() && (indentRightMM > 0.0))
            {
                styleStr += fmt::format("margin-right: {:.2f}mm; ", indentRightMM);
            }
            // First line indentation
            double firstLineIndentMM = - thisStyle.GetLeftSubIndent() / 10.0;
            if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) && (firstLineIndentMM > 0.0))
            {
                styleStr += fmt::format("text-indent: {:.2f}mm; ", firstLineIndentMM);
            }

            if (!styleStr.empty())
                str << " style=\"" << styleStr << "\"";

            str << ">";

            // TODO: convert to pixels
            int indentPixels = static_cast<int>(indentLeftMM*10/4);

            if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) == 0)
            {
                // Use a table to do indenting if we don't have CSS
                str << fmt::format("<table border=0 cellpadding=0 cellspacing=0><tr><td width=\"{:d}\"></td><td>", indentPixels);
                m_inTable = true;
            }

            if (((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) == 0) && (thisStyle.GetLeftSubIndent() < 0))
            {
                str << SymbolicIndent( - thisStyle.GetLeftSubIndent());
            }
        }
    }
    else
    {
        CloseLists(-1, str);

        std::string align = GetAlignment(thisStyle);
        str << fmt::format("<p align=\"{:s}\"", align.c_str());

        std::string styleStr;

        if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) && thisStyle.HasParagraphSpacingBefore())
        {
            double spacingBeforeMM = thisStyle.GetParagraphSpacingBefore() / 10.0;

            styleStr += fmt::format("margin-top: {:.2f}mm; ", spacingBeforeMM);
        }
        if ((GetFlags() & wxRICHTEXT_HANDLER_USE_CSS) && thisStyle.HasParagraphSpacingAfter())
        {
            double spacingAfterMM = thisStyle.GetParagraphSpacingAfter() / 10.0;

            styleStr += fmt::format("margin-bottom: {:.2f}mm; ", spacingAfterMM);
        }

        if (!styleStr.empty())
            str << " style=\"" << styleStr << "\"";

        str << ">";
    }
    OutputFont(thisStyle, str);
}

/// End paragraph formatting
void wxRichTextHTMLHandler::EndParagraphFormatting([[maybe_unused]] const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, wxTextOutputStream& stream)
{
    if (thisStyle.HasFont())
        stream << "</font>";

    if (m_inTable)
    {
        stream << "</td></tr></table></p>\n";
        m_inTable = false;
    }
    else if (!thisStyle.HasBulletStyle())
        stream << "</p>\n";
}

/// Closes lists to level (-1 means close all)
void wxRichTextHTMLHandler::CloseLists(int level, wxTextOutputStream& str)
{
    // Close levels high than this
    int i = m_indents.size()-1;
    while (i >= 0)
    {
        int l = m_indents[i];
        if (l > level)
        {
            if (m_listTypes[i] == 0)
                str << "</ol>";
            else
                str << "</ul>";
            m_indents.erase(std::begin(m_indents) + i);
            m_listTypes.erase(std::begin(m_listTypes) + i);
        }
        else
            break;
        i --;
     }
}

/// Output font tag
void wxRichTextHTMLHandler::OutputFont(const wxRichTextAttr& style, wxTextOutputStream& stream)
{
    if (style.HasFont())
    {
        stream << fmt::format("<font face=\"{:s}\" size=\"{:ld}\"", style.GetFontFaceName().c_str(), PtToSize(style.GetFontSize()));
        if (style.HasTextColour())
            stream << fmt::format(" color=\"{:s}\"", style.GetTextColour().GetAsString(wxC2S_HTML_SYNTAX).c_str());
        stream << " >";
    }
}

int wxRichTextHTMLHandler::TypeOfList( const wxRichTextAttr& thisStyle, std::string& tag )
{
    // We can use number attribute of li tag but not all the browsers support it.
    // also wxHtmlWindow doesn't support type attribute.

    bool m_is_ul = false;
    if (thisStyle.GetBulletStyle() == (wxTEXT_ATTR_BULLET_STYLE_ARABIC|wxTEXT_ATTR_BULLET_STYLE_PERIOD))
        tag = "<ol type=\"1\">";
    else if (thisStyle.GetBulletStyle() == wxTEXT_ATTR_BULLET_STYLE_LETTERS_UPPER)
        tag = "<ol type=\"A\">";
    else if (thisStyle.GetBulletStyle() == wxTEXT_ATTR_BULLET_STYLE_LETTERS_LOWER)
        tag = "<ol type=\"a\">";
    else if (thisStyle.GetBulletStyle() == wxTEXT_ATTR_BULLET_STYLE_ROMAN_UPPER)
        tag = "<ol type=\"I\">";
    else if (thisStyle.GetBulletStyle() == wxTEXT_ATTR_BULLET_STYLE_ROMAN_LOWER)
        tag = "<ol type=\"i\">";
    else
    {
        tag = "<ul>";
        m_is_ul = true;
    }

    if (m_is_ul)
        return 1;
    else
        return 0;
}

std::string wxRichTextHTMLHandler::GetAlignment( const wxRichTextAttr& thisStyle )
{
    switch( thisStyle.GetAlignment() )
    {
    case wxTextAttrAlignment::Left:
        return  "left";
    case wxTextAttrAlignment::Right:
        return "right";
    case wxTextAttrAlignment::Center:
        return "center";
    case wxTextAttrAlignment::Justified:
        return "justify";
    default:
        return "left";
    }
}

void wxRichTextHTMLHandler::WriteImage(wxRichTextImage* image, wxOutputStream& stream)
{
    wxTextOutputStream str(stream);

    str << "<img src=\"";

#if wxUSE_FILESYSTEM
    if (GetFlags() & wxRICHTEXT_HANDLER_SAVE_IMAGES_TO_MEMORY)
    {
#if 0
        if (!image->GetImage().IsOk() && image->GetImageBlock().GetData())
            image->LoadFromBlock();
        if (image->GetImage().IsOk() && !image->GetImageBlock().GetData())
            image->MakeBlock();
#endif

        if (image->GetImageBlock().IsOk())
        {
            wxImage img;
            image->GetImageBlock().Load(img);
            if (img.IsOk())
            {
                std::string ext(image->GetImageBlock().GetExtension());
                std::string tempFilename(fmt::format("image{:d}.{:s}", sm_fileCounter, ext.c_str()));
                wxMemoryFSHandler::AddFile(tempFilename, img, image->GetImageBlock().GetImageType());

                m_imageLocations.push_back(tempFilename);

                str << "memory:" << tempFilename;
            }
        }
        else
            str << "memory:?";

        sm_fileCounter ++;
    }
    else if (GetFlags() & wxRICHTEXT_HANDLER_SAVE_IMAGES_TO_FILES)
    {
#if 0
        if (!image->GetImage().IsOk() && image->GetImageBlock().GetData())
            image->LoadFromBlock();
        if (image->GetImage().IsOk() && !image->GetImageBlock().GetData())
            image->MakeBlock();
#endif

        if (image->GetImageBlock().IsOk())
        {
            std::string tempDir(GetTempDir());
            if (tempDir.empty())
                tempDir = wxFileName::GetTempDir();

            std::string ext(image->GetImageBlock().GetExtension());
            std::string tempFilename(fmt::format("{:s}/image{:d}.{:s}", tempDir.c_str(), sm_fileCounter, ext.c_str()));
            image->GetImageBlock().Write(tempFilename);

            m_imageLocations.push_back(tempFilename);

            // FIXME: stupid
            str << wxFileSystem::FileNameToURL(wxString(tempFilename));
        }
        else
            str << "file:?";

        sm_fileCounter ++;
    }
    else // if (GetFlags() & wxRICHTEXT_HANDLER_SAVE_IMAGES_TO_BASE64) // this is implied
#endif
    {
        str << "data:";
        str << GetMimeType(image->GetImageBlock().GetImageType());
        str << ";base64,";
#if 0
        if (image->GetImage().IsOk() && !image->GetImageBlock().GetData())
            image->MakeBlock();
#endif
        if (image->GetImageBlock().IsOk())
        {
            wxChar* data = b64enc( image->GetImageBlock().GetData(), image->GetImageBlock().GetDataSize() );
            str << data;

            delete[] data;
        }
    }

    str << "\" />";
}

long wxRichTextHTMLHandler::PtToSize(long size)
{
    int i;
    int len = m_fontSizeMapping.size();
    for (i = 0; i < len; i++)
        if (size <= m_fontSizeMapping[i])
            return i+1;
    return 7;
}

std::string wxRichTextHTMLHandler::SymbolicIndent(long indent)
{
    std::string in;
    for(;indent > 0; indent -= 20)
        in.append("&nbsp;");
    return in;
}

// FIXME: Convert to narrow string.
const wxChar* wxRichTextHTMLHandler::GetMimeType(wxBitmapType imageType)
{
    switch(imageType)
    {
    case wxBitmapType::BMP:
        return L"image/bmp";
    case wxBitmapType::TIFF:
        return L"image/tiff";
    case wxBitmapType::GIF:
        return L"image/gif";
    case wxBitmapType::PNG:
        return L"image/png";
    case wxBitmapType::JPEG:
        return L"image/jpeg";
    default:
        return L"image/unknown";
    }
}

// exim-style base64 encoder
wxChar* wxRichTextHTMLHandler::b64enc( unsigned char* input, size_t in_len )
{
    // elements of enc64 array must be 8 bit values
    // otherwise encoder will fail
    // hmmm.. Does wxT macro define a char as 16 bit value
    // when compiling with UNICODE option?
    static constexpr wxChar enc64[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    wxChar* output = new wxChar[4*((in_len+2)/3)+1];
    wxChar* p = output;

    while( in_len-- > 0 )
    {
        wxChar a, b;

        a = *input++;

        *p++ = enc64[ (a >> 2) & 0x3f ];

        if( in_len-- == 0 )
        {
            *p++ = enc64[ (a << 4 ) & 0x30 ];
            *p++ = '=';
            *p++ = '=';
            break;
        }

        b = *input++;

        *p++ = enc64[(( a << 4 ) | ((b >> 4) &0xf )) & 0x3f];

        if( in_len-- == 0 )
        {
            *p++ = enc64[ (b << 2) & 0x3f ];
            *p++ = '=';
            break;
        }

        a = *input++;

        *p++ = enc64[ ((( b << 2 ) & 0x3f ) | ((a >> 6)& 0x3)) & 0x3f ];

        *p++ = enc64[ a & 0x3f ];
    }
    *p = 0;

    return output;
}
#endif
// wxUSE_STREAMS

/// Delete the in-memory or temporary files generated by the last operation
bool wxRichTextHTMLHandler::DeleteTemporaryImages()
{
    return DeleteTemporaryImages(GetFlags(), m_imageLocations);
}

/// Delete the in-memory or temporary files generated by the last operation
bool wxRichTextHTMLHandler::DeleteTemporaryImages(int flags, const std::vector<std::string>& imageLocations)
{
    size_t i;
    for (i = 0; i < imageLocations.size(); i++)
    {
        std::string location = imageLocations[i];

        if (flags & wxRICHTEXT_HANDLER_SAVE_IMAGES_TO_MEMORY)
        {
#if wxUSE_FILESYSTEM
            wxMemoryFSHandler::RemoveFile(location);
#endif
        }
        else if (flags & wxRICHTEXT_HANDLER_SAVE_IMAGES_TO_FILES)
        {
            if (wxFileExists(location))
                wxRemoveFile(location);
        }
    }

    return true;
}


#endif
// wxUSE_RICHTEXT

