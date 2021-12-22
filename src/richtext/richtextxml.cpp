/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtextxml.cpp
// Purpose:     XML and HTML I/O for wxRichTextCtrl
// Author:      Julian Smart
// Modified by:
// Created:     2005-09-30
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#if wxUSE_RICHTEXT && wxUSE_XML

#include "wx/richtext/richtextxml.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/module.h"
    #include "wx/log.h"
#endif

#include "wx/filename.h"
#include "wx/clipbrd.h"
#include "wx/wfstream.h"
#include "wx/mstream.h"
#include "wx/stopwatch.h"
#include "wx/xml/xml.h"

import Utils.Strings;

import WX.Cmn.StrStream;
import WX.Cmn.TextStream;

// Set to 1 for slower wxXmlDocument method, 0 for faster direct method.
// If we make wxXmlDocument::Save more efficient, we might switch to this
// method.
#define wxRICHTEXT_USE_XMLDOCUMENT_OUTPUT 0

#if wxRICHTEXT_USE_XMLDOCUMENT_OUTPUT && !wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT
#   error Must define wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT in richtextxml.h to use this method.
#endif

#if !wxRICHTEXT_USE_XMLDOCUMENT_OUTPUT && !wxRICHTEXT_HAVE_DIRECT_OUTPUT
#   error Must define wxRICHTEXT_HAVE_DIRECT_OUTPUT in richtextxml.h to use this method.
#endif

// Set to 1 to time file saving
#define wxRICHTEXT_USE_OUTPUT_TIMINGS 0

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextXMLHandler, wxRichTextFileHandler);

#if wxUSE_STREAMS
bool wxRichTextXMLHandler::DoLoadFile(wxRichTextBuffer *buffer, wxInputStream& stream)
{
    if (!stream.IsOk())
        return false;

    m_helper.SetFlags(GetFlags());

    buffer->ResetAndClearCommands();
    buffer->Clear();

    wxXmlDocument* xmlDoc = new wxXmlDocument;
    bool success = true;

    // This is the encoding to convert to (memory encoding rather than file encoding)
    wxString encoding("UTF-8");

    if (!xmlDoc->Load(stream, encoding))
    {
        buffer->ResetAndClearCommands();
        success = false;
    }
    else
    {
        if (xmlDoc->GetRoot() && xmlDoc->GetRoot()->GetType() == wxXML_ELEMENT_NODE && xmlDoc->GetRoot()->GetName() == "richtext")
        {
            wxXmlNode* child = xmlDoc->GetRoot()->GetChildren();
            while (child)
            {
                if (child->GetType() == wxXML_ELEMENT_NODE)
                {
                    wxString name = child->GetName();
                    if (name == "richtext-version")
                    {
                    }
                    else
                        ImportXML(buffer, buffer, child);
                }

                child = child->GetNext();
            }
        }
        else
        {
            success = false;
        }
    }

    delete xmlDoc;

    buffer->UpdateRanges();

    return success;
}

/// Creates an object given an XML element name
wxRichTextObject* wxRichTextXMLHandler::CreateObjectForXMLName([[maybe_unused]] wxRichTextObject* parent, const wxString& name) const
{
    // The standard node to class mappings are added in wxRichTextModule::OnInit in richtextbuffer.cpp
    wxStringToStringHashMap::const_iterator it = sm_nodeNameToClassMap.find(name);
    if (it == sm_nodeNameToClassMap.end())
        return nullptr;
    else
        return wxDynamicCast(wxCreateDynamicObject(it->second), wxRichTextObject);
}

/// Recursively import an object
bool wxRichTextXMLHandler::ImportXML(wxRichTextBuffer* buffer, wxRichTextObject* obj, wxXmlNode* node)
{
    bool recurse = false;
    obj->ImportFromXML(buffer, node, this, & recurse);

    // TODO: how to control whether to import children.

    wxRichTextCompositeObject* compositeParent = wxDynamicCast(obj, wxRichTextCompositeObject);
    if (recurse && compositeParent)
    {
        wxXmlNode* child = node->GetChildren();
        while (child)
        {
            if (child->GetName() != "stylesheet")
            {
                wxRichTextObject* childObj = CreateObjectForXMLName(obj, child->GetName());
                if (childObj)
                {
                    compositeParent->AppendChild(childObj);
                    ImportXML(buffer, childObj, child);
                }
            }
            child = child->GetNext();
        }
    }

    return true;
}

bool wxRichTextXMLHandler::DoSaveFile(wxRichTextBuffer *buffer, wxOutputStream& stream)
{
    if (!stream.IsOk())
        return false;

    m_helper.SetupForSaving(m_encoding);
    m_helper.SetFlags(GetFlags());

    wxString version("1.0" ) ;

    wxString fileEncoding = m_helper.GetFileEncoding();

#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT && wxRICHTEXT_USE_XMLDOCUMENT_OUTPUT
#if wxRICHTEXT_USE_OUTPUT_TIMINGS
    wxStopWatch stopwatch;
#endif
    wxXmlDocument* doc = new wxXmlDocument;
    doc->SetFileEncoding(fileEncoding);

    wxXmlNode* rootNode = new wxXmlNode(wxXML_ELEMENT_NODE, "richtext");
    doc->SetRoot(rootNode);
    rootNode->AddAttribute("version"), wxT("1.0.0.0");
    rootNode->AddAttribute("xmlns"), wxT("http://www.wxwidgets.org");

    if (buffer->GetStyleSheet() && (GetFlags() & wxRICHTEXT_HANDLER_INCLUDE_STYLESHEET))
    {
        wxXmlNode* styleSheetNode = new wxXmlNode(wxXML_ELEMENT_NODE, "stylesheet");
        rootNode->AddChild(styleSheetNode);

        wxString nameAndDescr;

        if (!buffer->GetStyleSheet()->GetName().empty())
            styleSheetNode->AddAttribute("name", buffer->GetStyleSheet()->GetName());

        if (!buffer->GetStyleSheet()->GetDescription().empty())
            styleSheetNode->AddAttribute("description", buffer->GetStyleSheet()->GetDescription());

        int i;
        for (i = 0; i < (int) buffer->GetStyleSheet()->GetCharacterStyleCount(); i++)
        {
            wxRichTextCharacterStyleDefinition* def = buffer->GetStyleSheet()->GetCharacterStyle(i);
            m_helper.ExportStyleDefinition(styleSheetNode, def);
        }

        for (i = 0; i < (int) buffer->GetStyleSheet()->GetParagraphStyleCount(); i++)
        {
            wxRichTextParagraphStyleDefinition* def = buffer->GetStyleSheet()->GetParagraphStyle(i);
            m_helper.ExportStyleDefinition(styleSheetNode, def);
        }

        for (i = 0; i < (int) buffer->GetStyleSheet()->GetListStyleCount(); i++)
        {
            wxRichTextListStyleDefinition* def = buffer->GetStyleSheet()->GetListStyle(i);
            m_helper.ExportStyleDefinition(styleSheetNode, def);
        }

        for (i = 0; i < (int) buffer->GetStyleSheet()->GetBoxStyleCount(); i++)
        {
            wxRichTextBoxStyleDefinition* def = buffer->GetStyleSheet()->GetBoxStyle(i);
            m_helper.ExportStyleDefinition(styleSheetNode, def);
        }

        m_helper.WriteProperties(styleSheetNode, buffer->GetStyleSheet()->GetProperties());
    }
    bool success = ExportXML(rootNode, *buffer);
#if wxRICHTEXT_USE_OUTPUT_TIMINGS
    long t = stopwatch.Time();
    wxLogDebug("Creating the document took %ldms", t);
    wxMessageBox(wxString::Format("Creating the document took %ldms", t));
#endif
    if (success)
    {
#if wxRICHTEXT_USE_OUTPUT_TIMINGS
        wxStopWatch s2;
#endif
        success = doc->Save(stream);
#if wxRICHTEXT_USE_OUTPUT_TIMINGS
        long t2 = s2.Time();
        wxLogDebug("Save() took %ldms", t2);
        wxMessageBox(wxString::Format("Save() took %ldms", t2));
#endif
    }
    delete doc;
    doc = NULL;

#else
    // !(wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT && wxRICHTEXT_USE_XMLDOCUMENT_OUTPUT)

    wxString s ;
    s.Printf("<?xml version=\"%s\" encoding=\"%s\"?>\n",
             version.c_str(), fileEncoding.c_str());
    m_helper.OutputString(stream, s);
    m_helper.OutputString(stream, "<richtext version=\"1.0.0.0\" xmlns=\"http://www.wxwidgets.org\">");

    int level = 1;

    if (buffer->GetStyleSheet() && (GetFlags() & wxRICHTEXT_HANDLER_INCLUDE_STYLESHEET))
    {
        wxRichTextXMLHelper::OutputIndentation(stream, level);
        wxString nameAndDescr;
        if (!buffer->GetStyleSheet()->GetName().empty())
            nameAndDescr << " name=\"" << buffer->GetStyleSheet()->GetName() << "\"";
        if (!buffer->GetStyleSheet()->GetDescription().empty())
            nameAndDescr << " description=\"" << buffer->GetStyleSheet()->GetDescription() << "\"";
        m_helper.OutputString(stream, "<stylesheet" + nameAndDescr + ">");

        int i;

        for (i = 0; i < (int) buffer->GetStyleSheet()->GetCharacterStyleCount(); i++)
        {
            wxRichTextCharacterStyleDefinition* def = buffer->GetStyleSheet()->GetCharacterStyle(i);
            m_helper.ExportStyleDefinition(stream, def, level + 1);
        }

        for (i = 0; i < (int) buffer->GetStyleSheet()->GetParagraphStyleCount(); i++)
        {
            wxRichTextParagraphStyleDefinition* def = buffer->GetStyleSheet()->GetParagraphStyle(i);
            m_helper.ExportStyleDefinition(stream, def, level + 1);
        }

        for (i = 0; i < (int) buffer->GetStyleSheet()->GetListStyleCount(); i++)
        {
            wxRichTextListStyleDefinition* def = buffer->GetStyleSheet()->GetListStyle(i);
            m_helper.ExportStyleDefinition(stream, def, level + 1);
        }

        for (i = 0; i < (int) buffer->GetStyleSheet()->GetBoxStyleCount(); i++)
        {
            wxRichTextBoxStyleDefinition* def = buffer->GetStyleSheet()->GetBoxStyle(i);
            m_helper.ExportStyleDefinition(stream, def, level + 1);
        }

        m_helper.WriteProperties(stream, buffer->GetStyleSheet()->GetProperties(), level);

        wxRichTextXMLHelper::OutputIndentation(stream, level);
        m_helper.OutputString(stream, "</stylesheet>");
    }


    bool success = ExportXML(stream, *buffer, level);

    m_helper.OutputString(stream, "\n</richtext>");
    m_helper.OutputString(stream, "\n");
#endif

    return success;
}

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT

/// Recursively export an object
bool wxRichTextXMLHandler::ExportXML(wxOutputStream& stream, wxRichTextObject& obj, int indent)
{
    obj.ExportXML(stream, indent, this);

    return true;
}

#endif
    // wxRICHTEXT_HAVE_DIRECT_OUTPUT

#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT
bool wxRichTextXMLHandler::ExportXML(wxXmlNode* parent, wxRichTextObject& obj)
{
    obj.ExportXML(parent, this);

    return true;
}

#endif
    // wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT

#endif
    // wxUSE_STREAMS

// Import this object from XML
bool wxRichTextObject::ImportFromXML([[maybe_unused]] wxRichTextBuffer* buffer, wxXmlNode* node, wxRichTextXMLHandler* handler, bool* recurse)
{
    handler->GetHelper().ImportProperties(GetProperties(), node);
    handler->GetHelper().ImportStyle(GetAttributes(), node, UsesParagraphAttributes());

    wxString value = node->GetAttribute("show", "");
    if (!value.IsEmpty())
        Show(value == "1");

    *recurse = true;

    return true;
}

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
// Export this object directly to the given stream.
bool wxRichTextObject::ExportXML(wxOutputStream& stream, int indent, wxRichTextXMLHandler* handler)
{
    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    handler->GetHelper().OutputString(stream, "<" + GetXMLNodeName());

    wxString style = wxRichTextXMLHelper::AddAttributes(this, true);

    handler->GetHelper().OutputString(stream, style + ">");

    if (GetProperties().GetCount() > 0)
    {
        handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
    }

    wxRichTextCompositeObject* composite = wxDynamicCast(this, wxRichTextCompositeObject);
    if (composite)
    {
        size_t i;
        for (i = 0; i < composite->GetChildCount(); i++)
        {
            wxRichTextObject* child = composite->GetChild(i);
            child->ExportXML(stream, indent+1, handler);
        }
    }

    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    handler->GetHelper().OutputString(stream, "</" + GetXMLNodeName() + ">");
    return true;
}
#endif

#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT
// Export this object to the given parent node, usually creating at least one child node.
bool wxRichTextObject::ExportXML(wxXmlNode* parent, wxRichTextXMLHandler* handler)
{
    wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, GetXMLNodeName());
    parent->AddChild(elementNode);
    wxRichTextXMLHelper::AddAttributes(elementNode, this, true);
    handler->GetHelper().WriteProperties(elementNode, GetProperties());

    wxRichTextCompositeObject* composite = wxDynamicCast(this, wxRichTextCompositeObject);
    if (composite)
    {
        size_t i;
        for (i = 0; i < composite->GetChildCount(); i++)
        {
            wxRichTextObject* child = composite->GetChild(i);
            child->ExportXML(elementNode, handler);
        }
    }
    return true;
}
#endif

// Import this object from XML
bool wxRichTextPlainText::ImportFromXML(wxRichTextBuffer* buffer, wxXmlNode* node, wxRichTextXMLHandler* handler, bool* recurse)
{
    wxRichTextObject::ImportFromXML(buffer, node, handler, recurse);

    if (node->GetName() == "text")
    {
        wxString text;
        wxXmlNode* textChild = node->GetChildren();

        // First skip past properties, if any.
        wxXmlNode* n = textChild;
        while (n)
        {
            // Skip past properties
            if ((n->GetType() == wxXML_ELEMENT_NODE) && n->GetName() == "properties")
            {
                textChild = n->GetNext();
                n = nullptr;

                // Skip past the whitespace after the properties
                while (textChild && (textChild->GetType() == wxXML_TEXT_NODE))
                {
                    wxString cText = textChild->GetContent();
                    cText.Trim(true);
                    cText.Trim(false);
                    if (!cText.IsEmpty())
                    {
                        textChild->SetContent(cText);
                        break;
                    }
                    else
                        textChild = textChild->GetNext();
                }

                break;
            }
            if (n)
                n = n->GetNext();
        }

        while (textChild)
        {
            if (textChild->GetType() == wxXML_TEXT_NODE ||
                textChild->GetType() == wxXML_CDATA_SECTION_NODE)
            {
                wxString text2 = textChild->GetContent();

                // Strip whitespace from end
                if (!text2.empty() && text2[text2.length()-1] == wxT('\n'))
                    text2 = text2.Mid(0, text2.length()-1);

                if (!text2.empty() && text2[0] == wxT('"'))
                    text2 = text2.Mid(1);
                if (!text2.empty() && text2[text2.length()-1] == wxT('"'))
                    text2 = text2.Mid(0, text2.length() - 1);

                text += text2;
            }
            textChild = textChild->GetNext();
        }

        SetText(text);
    }
    else if (node->GetName() == "symbol")
    {
        // This is a symbol that XML can't read in the normal way
        wxString text;
        wxXmlNode* textChild = node->GetChildren();
        while (textChild)
        {
            if (textChild->GetType() == wxXML_TEXT_NODE ||
                textChild->GetType() == wxXML_CDATA_SECTION_NODE)
            {
                wxString text2 = textChild->GetContent();
                text += text2;
            }
            textChild = textChild->GetNext();
        }

        wxString actualText;
        actualText << (wxChar) wxAtoi(text);
        SetText(actualText);
    }
    else
        return false;

    return true;
}

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
// Export this object directly to the given stream.
bool wxRichTextPlainText::ExportXML(wxOutputStream& stream, int indent, wxRichTextXMLHandler* handler)
{
    wxString style = wxRichTextXMLHelper::AddAttributes(this, false);

    int i;
    int last = 0;
    const wxString& text = GetText();
    int len = (int) text.Length();

    if (len == 0)
    {
        i = 0;
        wxRichTextXMLHelper::OutputIndentation(stream, indent);
        handler->GetHelper().OutputString(stream, "<text");
        handler->GetHelper().OutputString(stream, style + ">");
        if (GetProperties().GetCount() > 0)
        {
            handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
            wxRichTextXMLHelper::OutputIndentation(stream, indent);
        }
        handler->GetHelper().OutputString(stream, "</text>");
    }
    else for (i = 0; i < len; i++)
    {
        int c = (int) text[i];
        if (((c < 32 || c == 34) && /* c != 9 && */ c != 10 && c != 13)
            // XML ranges
            || (!(c >= 32 && c <= 55295) && !(c >= 57344 && c <= 65533))
            )
        {
            if (i > 0)
            {
                wxString fragment(text.Mid(last, i-last));
                if (!fragment.empty())
                {
                    wxRichTextXMLHelper::OutputIndentation(stream, indent);
                    handler->GetHelper().OutputString(stream, "<text");

                    handler->GetHelper().OutputString(stream, style + ">");

                    if (!fragment.empty() && (fragment[0] == wxT(' ') || fragment[fragment.length()-1] == wxT(' ')))
                    {
                        handler->GetHelper().OutputString(stream, "\"");
                        handler->GetHelper().OutputStringEnt(stream, fragment);
                        handler->GetHelper().OutputString(stream, "\"");
                    }
                    else
                        handler->GetHelper().OutputStringEnt(stream, fragment);

                    if (GetProperties().GetCount() > 0)
                    {
                        handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
                        wxRichTextXMLHelper::OutputIndentation(stream, indent);
                    }
                    handler->GetHelper().OutputString(stream, "</text>");
                }
            }

            // Output this character as a number in a separate tag, because XML can't cope
            // with entities below 32 except for 10 and 13
            last = i + 1;
            wxRichTextXMLHelper::OutputIndentation(stream, indent);
            handler->GetHelper().OutputString(stream, "<symbol");

            handler->GetHelper().OutputString(stream, style + ">");
            handler->GetHelper().OutputString(stream, wxString::Format("%d", c));

            if (GetProperties().GetCount() > 0)
            {
                handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
                wxRichTextXMLHelper::OutputIndentation(stream, indent);
            }
            handler->GetHelper().OutputString(stream, "</symbol>");
        }
    }

    wxString fragment;
    if (last == 0)
        fragment = text;
    else
        fragment = text.Mid(last, i-last);

    if (last < len)
    {
        wxRichTextXMLHelper::OutputIndentation(stream, indent);
        handler->GetHelper().OutputString(stream, "<text");

        handler->GetHelper().OutputString(stream, style + ">");

        if (GetProperties().GetCount() > 0)
        {
            handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
            wxRichTextXMLHelper::OutputIndentation(stream, indent);
        }

        if (!fragment.empty() && (fragment[0] == wxT(' ') || fragment[fragment.length()-1] == wxT(' ')))
        {
            handler->GetHelper().OutputString(stream, "\"");
            handler->GetHelper().OutputStringEnt(stream, fragment);
            handler->GetHelper().OutputString(stream, "\"");
        }
        else
            handler->GetHelper().OutputStringEnt(stream, fragment);

        handler->GetHelper().OutputString(stream, "</text>");
    }
    return true;
}
#endif

#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT
// Export this object to the given parent node, usually creating at least one child node.
bool wxRichTextPlainText::ExportXML(wxXmlNode* parent, wxRichTextXMLHandler* handler)
{
    int i;
    int last = 0;
    const wxString& text = GetText();
    int len = (int) text.Length();

    if (len == 0)
    {
        i = 0;

        wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, "text");
        parent->AddChild(elementNode);

        wxRichTextXMLHelper::AddAttributes(elementNode, GetAttributes(), false);
        handler->GetHelper().WriteProperties(elementNode, GetProperties());
    }
    else for (i = 0; i < len; i++)
    {
        int c = (int) text[i];

        if ((c < 32 || c == 34) && c != 10 && c != 13)
        {
            if (i > 0)
            {
                wxString fragment(text.Mid(last, i-last));
                if (!fragment.empty())
                {
                    // TODO: I'm assuming wxXmlDocument will output quotes if necessary
                    wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, "text");
                    parent->AddChild(elementNode);
                    wxRichTextXMLHelper::AddAttributes(elementNode, GetAttributes(), false);
                    handler->GetHelper().WriteProperties(elementNode, GetProperties());

                    wxXmlNode* textNode = new wxXmlNode(wxXML_TEXT_NODE, "text");
                    elementNode->AddChild(textNode);

                    if (fragment[0] == wxT(' ') || fragment[fragment.length()-1] == wxT(' '))
                        fragment = "\"" + fragment + "\"";

                    textNode->SetContent(fragment);
                }
            }

            // Output this character as a number in a separate tag, because XML can't cope
            // with entities below 32 except for 10 and 13

            wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, "symbol");
            parent->AddChild(elementNode);

            wxRichTextXMLHelper::AddAttributes(elementNode, GetAttributes(), false);
            handler->GetHelper().WriteProperties(elementNode, GetProperties());

            wxXmlNode* textNode = new wxXmlNode(wxXML_TEXT_NODE, "text");
            elementNode->AddChild(textNode);
            textNode->SetContent(wxString::Format("%d", c));

            last = i + 1;
        }
    }

    wxString fragment;
    if (last == 0)
        fragment = text;
    else
        fragment = text.Mid(last, i-last);

    if (last < len)
    {
        wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, "text");
        parent->AddChild(elementNode);
        wxRichTextXMLHelper::AddAttributes(elementNode, GetAttributes(), false);

        wxXmlNode* textNode = new wxXmlNode(wxXML_TEXT_NODE, "text");
        elementNode->AddChild(textNode);

        if (fragment[0] == wxT(' ') || fragment[fragment.length()-1] == wxT(' '))
            fragment = "\"" + fragment + "\"";

        textNode->SetContent(fragment);
    }
    return true;
}
#endif

// Import this object from XML
bool wxRichTextImage::ImportFromXML(wxRichTextBuffer* buffer, wxXmlNode* node, wxRichTextXMLHandler* handler, bool* recurse)
{
    wxRichTextObject::ImportFromXML(buffer, node, handler, recurse);

    wxBitmapType imageType = wxBitmapType::PNG;
    wxString value = node->GetAttribute("imagetype", "");
    if (!value.empty())
    {
        int type = wxAtoi(value);

        // note: 0 == wxBitmapType::Invalid
        if (type <= 0 || type >= static_cast<int>(wxBitmapType::Max))
        {
            wxLogWarning("Invalid bitmap type specified for <image> tag: %d", type);
        }
        else
        {
            imageType = (wxBitmapType)type;
        }
    }

    wxString data;

    wxXmlNode* imageChild = node->GetChildren();
    while (imageChild)
    {
        wxString childName = imageChild->GetName();
        if (childName == "data")
        {
            wxXmlNode* dataChild = imageChild->GetChildren();
            while (dataChild)
            {
                data = dataChild->GetContent();
                // wxLogDebug(data);
                dataChild = dataChild->GetNext();
            }

        }
        imageChild = imageChild->GetNext();
    }

    if (!data.empty())
    {
        wxStringInputStream strStream(data);

        GetImageBlock().ReadHex(strStream, data.length(), imageType);

        return true;
    }
    else
        return false;
}

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
// Export this object directly to the given stream.
bool wxRichTextImage::ExportXML(wxOutputStream& stream, int indent, wxRichTextXMLHandler* handler)
{
    wxString style = wxRichTextXMLHelper::AddAttributes(this, false);

    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    handler->GetHelper().OutputString(stream, "<image");
    if (!GetImageBlock().IsOk())
    {
        // No data
        handler->GetHelper().OutputString(stream, style + ">");
    }
    else
    {
        handler->GetHelper().OutputString(stream, wxString::Format(" imagetype=\"%d\"", (int) GetImageBlock().GetImageType()) + style + ">");
    }
    if (GetProperties().GetCount() > 0)
    {
        handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
        wxRichTextXMLHelper::OutputIndentation(stream, indent);
    }

    wxRichTextXMLHelper::OutputIndentation(stream, indent+1);
    handler->GetHelper().OutputString(stream, "<data>");

    // wxStopWatch stopwatch;

    GetImageBlock().WriteHex(stream);

    // wxLogDebug("Image conversion to hex took %ldms", stopwatch.Time());

    handler->GetHelper().OutputString(stream, "</data>\n");
    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    handler->GetHelper().OutputString(stream, "</image>");
    return true;
}
#endif

#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT
// Export this object to the given parent node, usually creating at least one child node.
bool wxRichTextImage::ExportXML(wxXmlNode* parent, wxRichTextXMLHandler* handler)
{
    wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, "image");
    parent->AddChild(elementNode);

    if (GetImageBlock().IsOk())
        elementNode->AddAttribute("imagetype", wxRichTextXMLHelper::MakeString((int) GetImageBlock().GetImageType()));

    wxRichTextXMLHelper::AddAttributes(elementNode, this, false);
    handler->GetHelper().WriteProperties(elementNode, GetProperties());

    wxXmlNode* dataNode = new wxXmlNode(wxXML_ELEMENT_NODE, "data");
    elementNode->AddChild(dataNode);
    wxXmlNode* textNode = new wxXmlNode(wxXML_TEXT_NODE, "text");
    dataNode->AddChild(textNode);

    wxString strData;
#if 1
    {
        wxMemoryOutputStream stream;
        if (GetImageBlock().WriteHex(stream))
        {
            if (stream.GetSize() > 0)
            {
                const auto size = stream.GetSize();
#ifdef __WXDEBUG__
                const auto size2 = stream.GetOutputStreamBuffer()->GetIntPosition();
                wxASSERT(size == size2);
#endif
                unsigned char* data = new unsigned char[size];
                stream.CopyTo(data, size);
                strData = wxString((const char*) data, wxConvUTF8, size);
                delete[] data;
            }
        }

    }
#else
    {
        wxStringOutputStream strStream(& strData);
        GetImageBlock().WriteHex(strStream);
    }
#endif

    textNode->SetContent(strData);
#if wxCHECK_VERSION(2,9,0)
    textNode->SetNoConversion(true); // optimize speed
#endif

    return true;
}
#endif

// Import this object from XML
bool wxRichTextParagraphLayoutBox::ImportFromXML(wxRichTextBuffer* buffer, wxXmlNode* node, wxRichTextXMLHandler* handler, bool* recurse)
{
    wxRichTextObject::ImportFromXML(buffer, node, handler, recurse);

    *recurse = true;

    wxString partial = node->GetAttribute("partialparagraph", "");
    if (partial == "true")
        SetPartialParagraph(true);

    wxXmlNode* child = handler->GetHelper().FindNode(node, "stylesheet");
    if (child && (handler->GetFlags() & wxRICHTEXT_HANDLER_INCLUDE_STYLESHEET))
    {
        wxRichTextStyleSheet* sheet = new wxRichTextStyleSheet;
        wxString sheetName = child->GetAttribute("name", "");
        wxString sheetDescription = child->GetAttribute("description", "");
        sheet->SetName(sheetName);
        sheet->SetDescription(sheetDescription);

        wxXmlNode* child2 = child->GetChildren();
        while (child2)
        {
            handler->GetHelper().ImportStyleDefinition(sheet, child2);

            child2 = child2->GetNext();
        }
        handler->GetHelper().ImportProperties(sheet->GetProperties(), child);

        // Notify that styles have changed. If this is vetoed by the app,
        // the new sheet will be deleted. If it is not vetoed, the
        // old sheet will be deleted and replaced with the new one.
        buffer->SetStyleSheetAndNotify(sheet);
    }

    return true;
}

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
// Export this object directly to the given stream.
bool wxRichTextParagraphLayoutBox::ExportXML(wxOutputStream& stream, int indent, wxRichTextXMLHandler* handler)
{
    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    wxString nodeName = GetXMLNodeName();
    handler->GetHelper().OutputString(stream, "<" + nodeName);

    wxString style = wxRichTextXMLHelper::AddAttributes(this, true);

    if (GetPartialParagraph())
        style << " partialparagraph=\"true\"";

    handler->GetHelper().OutputString(stream, style + ">");

    if (GetProperties().GetCount() > 0)
    {
        handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
    }

    size_t i;
    for (i = 0; i < GetChildCount(); i++)
    {
        wxRichTextObject* child = GetChild(i);
        child->ExportXML(stream, indent+1, handler);
    }

    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    handler->GetHelper().OutputString(stream, "</" + nodeName + ">");
    return true;
}
#endif

#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT
// Export this object to the given parent node, usually creating at least one child node.
bool wxRichTextParagraphLayoutBox::ExportXML(wxXmlNode* parent, wxRichTextXMLHandler* handler)
{
    wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, GetXMLNodeName());
    parent->AddChild(elementNode);
    wxRichTextXMLHelper::AddAttributes(elementNode, this, true);
    handler->GetHelper().WriteProperties(elementNode, GetProperties());

    if (GetPartialParagraph())
        elementNode->AddAttribute("partialparagraph", "true");

    size_t i;
    for (i = 0; i < GetChildCount(); i++)
    {
        wxRichTextObject* child = GetChild(i);
        child->ExportXML(elementNode, handler);
    }

    return true;
}
#endif

// Import this object from XML
bool wxRichTextTable::ImportFromXML(wxRichTextBuffer* buffer, wxXmlNode* node, wxRichTextXMLHandler* handler, bool* recurse)
{
    wxRichTextBox::ImportFromXML(buffer, node, handler, recurse);

    *recurse = false;

    m_rowCount = wxAtoi(node->GetAttribute("rows", ""));
    m_colCount = wxAtoi(node->GetAttribute("cols", ""));

    wxXmlNode* child = node->GetChildren();
    while (child)
    {
        wxRichTextObject* childObj = handler->CreateObjectForXMLName(this, child->GetName());
        if (childObj)
        {
            AppendChild(childObj);
            handler->ImportXML(buffer, childObj, child);
        }
        child = child->GetNext();
    }

    m_cells.Add(wxRichTextObjectPtrArray(), m_rowCount);
    int i, j;
    for (i = 0; i < m_rowCount; i++)
    {
        wxRichTextObjectPtrArray& colArray = m_cells[i];
        for (j = 0; j < m_colCount; j++)
        {
            int idx = i * m_colCount + j;
            if (idx < (int) GetChildren().GetCount())
            {
                wxRichTextCell* cell = wxDynamicCast(GetChildren().Item(idx)->GetData(), wxRichTextCell);
                if (cell)
                    colArray.Add(cell);
            }
        }
    }

    return true;
}

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
// Export this object directly to the given stream.
bool wxRichTextTable::ExportXML(wxOutputStream& stream, int indent, wxRichTextXMLHandler* handler)
{
    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    wxString nodeName = GetXMLNodeName();
    handler->GetHelper().OutputString(stream, "<" + nodeName);

    wxString style = wxRichTextXMLHelper::AddAttributes(this, true);

    style << " rows=\"" << m_rowCount << "\"";
    style << " cols=\"" << m_colCount << "\"";

    handler->GetHelper().OutputString(stream, style + ">");

    if (GetProperties().GetCount() > 0)
    {
        handler->GetHelper().WriteProperties(stream, GetProperties(), indent);
    }

    int i, j;
    for (i = 0; i < m_rowCount; i++)
    {
        for (j = 0; j < m_colCount; j ++)
        {
            wxRichTextCell* cell = GetCell(i, j);
            cell->ExportXML(stream, indent+1, handler);
        }
    }

    wxRichTextXMLHelper::OutputIndentation(stream, indent);
    handler->GetHelper().OutputString(stream, "</" + nodeName + ">");

    return true;
}
#endif

#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT
// Export this object to the given parent node, usually creating at least one child node.
bool wxRichTextTable::ExportXML(wxXmlNode* parent, wxRichTextXMLHandler* handler)
{
    wxXmlNode* elementNode = new wxXmlNode(wxXML_ELEMENT_NODE, GetXMLNodeName());
    parent->AddChild(elementNode);
    wxRichTextXMLHelper::AddAttributes(elementNode, this, true);
    handler->GetHelper().WriteProperties(elementNode, GetProperties());

    elementNode->AddAttribute("rows", wxString::Format("%d", m_rowCount));
    elementNode->AddAttribute("cols", wxString::Format("%d", m_colCount));

    int i, j;
    for (i = 0; i < m_rowCount; i++)
    {
        for (j = 0; j < m_colCount; j ++)
        {
            wxRichTextCell* cell = GetCell(i, j);
            cell->ExportXML(elementNode, handler);
        }
    }

    return true;
}
#endif

wxRichTextXMLHelper::~wxRichTextXMLHelper()
{
    Clear();
}

void wxRichTextXMLHelper::Clear()
{
#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
    if (m_deleteConvFile)
        delete m_convFile;
    m_convFile = nullptr;
    m_convMem = nullptr;
    m_deleteConvFile = false;
#endif
    m_fileEncoding.clear();
}

void wxRichTextXMLHelper::SetupForSaving(const wxString& enc)
{
    Clear();

    m_fileEncoding = "UTF-8";
#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
    m_convFile = & wxConvUTF8;
#endif

    // If we pass an explicit encoding, change the output encoding.
    if (!enc.empty() && enc.Lower() != m_fileEncoding.Lower())
    {
        if (enc == "<System>")
        {
#if wxUSE_INTL
            m_fileEncoding = wxLocale::GetSystemEncodingName();
            // if !wxUSE_INTL, we fall back to UTF-8 or ISO-8859-1 below
#endif
        }
        else
        {
            m_fileEncoding = enc;
        }

        // GetSystemEncodingName may not have returned a name
        if (m_fileEncoding.empty())
            m_fileEncoding = "UTF-8";
#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
        m_convFile = new wxCSConv(m_fileEncoding);
        m_deleteConvFile = true;
#endif
    }

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
    m_convMem = nullptr;
#endif
}

// Convert a colour to a 6-digit hex string
wxString wxRichTextXMLHelper::ColourToHexString(const wxColour& col)
{
    wxString hex;

    hex += wxDecToHex(col.Red());
    hex += wxDecToHex(col.Green());
    hex += wxDecToHex(col.Blue());

    return hex;
}

// Convert 6-digit hex string to a colour
wxColour wxRichTextXMLHelper::HexStringToColour(const wxString& hex)
{
    unsigned char r = (unsigned char)wxHexToDec(hex.Mid(0, 2));
    unsigned char g = (unsigned char)wxHexToDec(hex.Mid(2, 2));
    unsigned char b = (unsigned char)wxHexToDec(hex.Mid(4, 2));

    return {r, g, b};
}

//-----------------------------------------------------------------------------
//  xml support routines
//-----------------------------------------------------------------------------

bool wxRichTextXMLHelper::HasParam(wxXmlNode* node, const wxString& param)
{
    return (GetParamNode(node, param) != nullptr);
}

wxXmlNode *wxRichTextXMLHelper::GetParamNode(wxXmlNode* node, const wxString& param)
{
    wxCHECK_MSG(node, nullptr, "You can't access node data before it was initialized!");

    wxXmlNode *n = node->GetChildren();

    while (n)
    {
        if (n->GetType() == wxXML_ELEMENT_NODE && n->GetName() == param)
            return n;
        n = n->GetNext();
    }
    return nullptr;
}

wxString wxRichTextXMLHelper::GetNodeContent(wxXmlNode *node)
{
    wxXmlNode *n = node;
    if (n == nullptr) return {};
    n = n->GetChildren();

    while (n)
    {
        if (n->GetType() == wxXML_TEXT_NODE ||
            n->GetType() == wxXML_CDATA_SECTION_NODE)
            return n->GetContent();
        n = n->GetNext();
    }
    return {};
}

wxString wxRichTextXMLHelper::GetParamValue(wxXmlNode *node, const wxString& param)
{
    if (param.empty())
        return GetNodeContent(node);
    else
        return GetNodeContent(GetParamNode(node, param));
}

wxString wxRichTextXMLHelper::GetText(wxXmlNode *node, const wxString& param)
{
    wxXmlNode *parNode = GetParamNode(node, param);
    if (!parNode)
        parNode = node;
    wxString str1(GetNodeContent(parNode));
    return str1;
}

wxXmlNode* wxRichTextXMLHelper::FindNode(wxXmlNode* node, const wxString& name)
{
    if (node->GetName() == name && name == "stylesheet")
        return node;

    wxXmlNode* child = node->GetChildren();
    while (child)
    {
        if (child->GetName() == name)
            return child;
        child = child->GetNext();
    }
    return nullptr;
}

wxString wxRichTextXMLHelper::AttributeToXML(const wxString& str)
{
    wxString str1;
    size_t i, last, len;
    wxChar c;

    len = str.Len();
    last = 0;
    for (i = 0; i < len; i++)
    {
        c = str.GetChar(i);

        // Original code excluded "&amp;" but we _do_ want to convert
        // the ampersand beginning &amp; because otherwise when read in,
        // the original "&amp;" becomes "&".

        if (c == wxT('<') || c == wxT('>') || c == wxT('"') ||
            (c == wxT('&') /* && (str.Mid(i+1, 4) != "amp;") */ ))
        {
            str1 += str.Mid(last, i - last);
            switch (c)
            {
            case wxT('<'):
                str1 += "&lt;";
                break;
            case wxT('>'):
                str1 += "&gt;";
                break;
            case wxT('&'):
                str1 += "&amp;";
                break;
            case wxT('"'):
                str1 += "&quot;";
                break;
            default: break;
            }
            last = i + 1;
        }
        else if (wxUChar(c) > 127)
        {
            str1 += str.Mid(last, i - last);

            wxString s("&#");
            s << (int) c;
            s << ";";
            str1 += s;
            last = i + 1;
        }
    }
    str1 += str.Mid(last, i - last);
    return str1;
}

// Make a string from the given property. This can be overridden for custom variants.
wxString wxRichTextXMLHelper::MakeStringFromProperty(const wxVariant& var)
{
    return var.MakeString();
}

// Create a proprty from the string read from the XML file.
wxVariant wxRichTextXMLHelper::MakePropertyFromString(const wxString& name, const wxString& value, [[maybe_unused]] const wxString& type)
{
    wxVariant var(value, name);
    // TODO: use type to create using common types
    return var;
}

/// Replace face name with current name for platform.
/// TODO: introduce a virtual function or settable table to
/// do this comprehensively.
bool wxRichTextXMLHelper::RichTextFixFaceName(wxString& facename)
{
    if (facename.empty())
        return false;

#ifdef __WXMSW__
    if (facename == "Times")
    {
        facename = "Times New Roman";
        return true;
    }
    else if (facename == "Helvetica")
    {
        facename = "Arial";
        return true;
    }
    else if (facename == "Courier")
    {
        facename = "Courier New";
        return true;
    }
    else
        return false;
#else
    if (facename == "Times New Roman")
    {
        facename = "Times";
        return true;
    }
    else if (facename == "Arial")
    {
        facename = "Helvetica";
        return true;
    }
    else if (facename == "Courier New")
    {
        facename = "Courier";
        return true;
    }
    else
        return false;
#endif
}

long wxRichTextXMLHelper::ColourStringToLong(const wxString& colStr)
{
    if (!colStr.IsEmpty())
    {
        wxColour col(colStr);
#if wxCHECK_VERSION(2,9,0)
        return col.GetRGB();
#else
        return (col.Red() | (col.Green() << 8) | (col.Blue() << 16));
#endif
    }
    else
        return 0;
}

wxTextAttrDimension wxRichTextXMLHelper::ParseDimension(const wxString& dimStr)
{
    wxString valuePart = dimStr.BeforeFirst(wxT(','));
    wxString flagsPart;
    if (dimStr.Contains(","))
        flagsPart = dimStr.AfterFirst(wxT(','));
    wxTextAttrDimension dim;
    dim.SetValue(wxAtoi(valuePart));
    dim.SetFlags(wxAtoi(flagsPart));

    return dim;
}

/// Import style parameters
bool wxRichTextXMLHelper::ImportStyle(wxRichTextAttr& attr, wxXmlNode* node, bool isPara)
{
    wxXmlAttribute* xmlAttr = node->GetAttributes();
    bool found;
    while (xmlAttr)
    {
        const wxString& name = xmlAttr->GetName();
        const wxString& value = xmlAttr->GetValue();
        found = true;

        if (name == "fontface")
        {
            if (!value.empty())
            {
                wxString v = value;
                if (GetFlags() & wxRICHTEXT_HANDLER_CONVERT_FACENAMES)
                    RichTextFixFaceName(v);
                attr.SetFontFaceName(v);
            }
        }
        else if (name == "fontfamily")
        {
            if (!value.empty())
                attr.SetFontFamily((wxFontFamily)wxAtoi(value));
        }
        else if (name == "fontstyle")
        {
            if (!value.empty())
                attr.SetFontStyle((wxFontStyle)wxAtoi(value));
        }
        else if (name == "fontsize" || name == "fontpointsize")
        {
            if (!value.empty())
                attr.SetFontPointSize(wxAtoi(value));
        }
        else if (name == "fontpixelsize")
        {
            if (!value.empty())
                attr.SetFontPixelSize(wxAtoi(value));
        }
        else if (name == "fontweight")
        {
            if (!value.empty())
                attr.SetFontWeight((wxFontWeight) wxAtoi(value));
        }
        else if (name == "fontunderlined")
        {
            if (!value.empty())
                attr.SetFontUnderlined(wxAtoi(value) != 0);
        }
        else if (name == "textcolor")
        {
            if (!value.empty())
            {
                if (value[0] == wxT('#'))
                    attr.SetTextColour(HexStringToColour(value.Mid(1)));
                else
                    attr.SetTextColour(value);
            }
        }
        else if (name == "bgcolor")
        {
            if (!value.empty())
            {
                if (value[0] == wxT('#'))
                    attr.SetBackgroundColour(HexStringToColour(value.Mid(1)));
                else
                    attr.SetBackgroundColour(value);
            }
        }
        else if (name == "characterstyle")
        {
            if (!value.empty())
                attr.SetCharacterStyleName(value);
        }
        else if (name == "texteffects")
        {
            if (!value.empty())
                attr.SetTextEffects(wxAtoi(value));
        }
        else if (name == "texteffectflags")
        {
            if (!value.empty())
                attr.SetTextEffectFlags(wxAtoi(value));
        }
        else if (name == "url")
        {
            if (!value.empty())
                attr.SetURL(value);
        }
        else if (isPara)
        {
            if (name == "alignment")
            {
                if (!value.empty())
                    attr.SetAlignment((wxTextAttrAlignment) wxAtoi(value));
            }
            else if (name == "leftindent")
            {
                if (!value.empty())
                    attr.SetLeftIndent(wxAtoi(value), attr.GetLeftSubIndent());
            }
            else if (name == "leftsubindent")
            {
                if (!value.empty())
                    attr.SetLeftIndent(attr.GetLeftIndent(), wxAtoi(value));
            }
            else if (name == "rightindent")
            {
                if (!value.empty())
                    attr.SetRightIndent(wxAtoi(value));
            }
            else if (name == "parspacingbefore")
            {
                if (!value.empty())
                    attr.SetParagraphSpacingBefore(wxAtoi(value));
            }
            else if (name == "parspacingafter")
            {
                if (!value.empty())
                    attr.SetParagraphSpacingAfter(wxAtoi(value));
            }
            else if (name == "linespacing")
            {
                if (!value.empty())
                    attr.SetLineSpacing(wxAtoi(value));
            }
            else if (name == "bulletstyle")
            {
                if (!value.empty())
                    attr.SetBulletStyle(wxAtoi(value));
            }
            else if (name == "bulletnumber")
            {
                if (!value.empty())
                    attr.SetBulletNumber(wxAtoi(value));
            }
            else if (name == "bulletsymbol")
            {
                if (!value.empty())
                {
                    wxChar ch = wxAtoi(value);
                    wxString s;
                    s << ch;
                    attr.SetBulletText(s);
                }
            }
            else if (name == "bullettext")
            {
                if (!value.empty())
                {
                    attr.SetBulletText(value);
                }
            }
            else if (name == "bulletfont")
            {
                if (!value.empty())
                {
                    attr.SetBulletFont(value);
                }
            }
            else if (name == "bulletname")
            {
                if (!value.empty())
                {
                    attr.SetBulletName(value);
                }
            }
            else if (name == "parstyle")
            {
                if (!value.empty())
                {
                    attr.SetParagraphStyleName(value);
                }
            }
            else if (name == "liststyle")
            {
                if (!value.empty())
                {
                    attr.SetListStyleName(value);
                }
            }
            else if (name == "boxstyle")
            {
                if (!value.empty())
                {
                    attr.GetTextBoxAttr().SetBoxStyleName(value);
                }
            }
            else if (name == "tabs")
            {
                if (!value.empty())
                {
                    std::vector<int> tabs;
                    wxStringTokenizer tkz(value, ",");
                    while (tkz.HasMoreTokens())
                    {
                        wxString token = tkz.GetNextToken();
                        tabs.push_back(wxAtoi(token));
                    }
                    attr.SetTabs(tabs);
                }
            }
            else if (name == "pagebreak")
            {
                if (!value.empty())
                {
                    attr.SetPageBreak(wxAtoi(value) != 0);
                }
            }
            else if (name == "outlinelevel")
            {
                if (!value.empty())
                {
                    attr.SetOutlineLevel(wxAtoi(value));
                }
            }
            else
                found = false;
        }
        else
            found = false;

        if (!found)
        {
            // Box attributes

            if (name == "width")
            {
                attr.GetTextBoxAttr().GetWidth().SetValue(ParseDimension(value));
            }
            else if (name == "height")
            {
                attr.GetTextBoxAttr().GetHeight().SetValue(ParseDimension(value));
            }
            else if (name == "minwidth")
            {
                attr.GetTextBoxAttr().GetMinSize().GetWidth().SetValue(ParseDimension(value));
            }
            else if (name == "minheight")
            {
                attr.GetTextBoxAttr().GetMinSize().GetHeight().SetValue(ParseDimension(value));
            }
            else if (name == "maxwidth")
            {
                attr.GetTextBoxAttr().GetMaxSize().GetWidth().SetValue(ParseDimension(value));
            }
            else if (name == "maxheight")
            {
                attr.GetTextBoxAttr().GetMaxSize().GetHeight().SetValue(ParseDimension(value));
            }
            else if (name == "corner-radius")
            {
                attr.GetTextBoxAttr().SetCornerRadius(ParseDimension(value));
            }

            else if (name == "verticalalignment")
            {
                if (value == "top")
                    attr.GetTextBoxAttr().SetVerticalAlignment(wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_TOP);
                else if (value == "centre")
                    attr.GetTextBoxAttr().SetVerticalAlignment(wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_CENTRE);
                else if (value == "bottom")
                    attr.GetTextBoxAttr().SetVerticalAlignment(wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_BOTTOM);
                else if (value == "none")
                    attr.GetTextBoxAttr().SetVerticalAlignment(wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_NONE);
            }
            else if (name == "float")
            {
                if (value == "left")
                    attr.GetTextBoxAttr().SetFloatMode(wxTEXT_BOX_ATTR_FLOAT_LEFT);
                else if (value == "right")
                    attr.GetTextBoxAttr().SetFloatMode(wxTEXT_BOX_ATTR_FLOAT_RIGHT);
                else if (value == "none")
                    attr.GetTextBoxAttr().SetFloatMode(wxTEXT_BOX_ATTR_FLOAT_NONE);
            }
            else if (name == "clear")
            {
                if (value == "left")
                    attr.GetTextBoxAttr().SetClearMode(wxTEXT_BOX_ATTR_CLEAR_LEFT);
                else if (value == "right")
                    attr.GetTextBoxAttr().SetClearMode(wxTEXT_BOX_ATTR_CLEAR_RIGHT);
                else if (value == "both")
                    attr.GetTextBoxAttr().SetClearMode(wxTEXT_BOX_ATTR_CLEAR_BOTH);
                else if (value == "none")
                    attr.GetTextBoxAttr().SetClearMode(wxTEXT_BOX_ATTR_CLEAR_NONE);
            }
            else if (name == "collapse-borders")
                attr.GetTextBoxAttr().SetCollapseBorders((wxTextBoxAttrCollapseMode) wxAtoi(value));
            else if (name == "whitespace-mode")
                attr.GetTextBoxAttr().SetWhitespaceMode((wxTextBoxAttrWhitespaceMode) wxAtoi(value));

            else if (name.Contains("border-"))
            {
                if (name == "border-left-style")
                    attr.GetTextBoxAttr().GetBorder().GetLeft().SetStyle(wxAtoi(value));
                else if (name == "border-right-style")
                    attr.GetTextBoxAttr().GetBorder().GetRight().SetStyle(wxAtoi(value));
                else if (name == "border-top-style")
                    attr.GetTextBoxAttr().GetBorder().GetTop().SetStyle(wxAtoi(value));
                else if (name == "border-bottom-style")
                    attr.GetTextBoxAttr().GetBorder().GetBottom().SetStyle(wxAtoi(value));

                else if (name == "border-left-color")
                    attr.GetTextBoxAttr().GetBorder().GetLeft().SetColour(ColourStringToLong(value));
                else if (name == "border-right-color")
                    attr.GetTextBoxAttr().GetBorder().GetRight().SetColour(ColourStringToLong(value));
                else if (name == "border-top-color")
                    attr.GetTextBoxAttr().GetBorder().GetTop().SetColour(ColourStringToLong(value));
                else if (name == "border-bottom-color")
                    attr.GetTextBoxAttr().GetBorder().GetBottom().SetColour(ColourStringToLong(value));

                else if (name == "border-left-width")
                    attr.GetTextBoxAttr().GetBorder().GetLeft().SetWidth(ParseDimension(value));
                else if (name == "border-right-width")
                    attr.GetTextBoxAttr().GetBorder().GetRight().SetWidth(ParseDimension(value));
                else if (name == "border-top-width")
                    attr.GetTextBoxAttr().GetBorder().GetTop().SetWidth(ParseDimension(value));
                else if (name == "border-bottom-width")
                    attr.GetTextBoxAttr().GetBorder().GetBottom().SetWidth(ParseDimension(value));
            }
            else if (name.Contains("outline-"))
            {
                if (name == "outline-left-style")
                    attr.GetTextBoxAttr().GetOutline().GetLeft().SetStyle(wxAtoi(value));
                else if (name == "outline-right-style")
                    attr.GetTextBoxAttr().GetOutline().GetRight().SetStyle(wxAtoi(value));
                else if (name == "outline-top-style")
                    attr.GetTextBoxAttr().GetOutline().GetTop().SetStyle(wxAtoi(value));
                else if (name == "outline-bottom-style")
                    attr.GetTextBoxAttr().GetOutline().GetBottom().SetStyle(wxAtoi(value));

                else if (name == "outline-left-color")
                    attr.GetTextBoxAttr().GetOutline().GetLeft().SetColour(ColourStringToLong(value));
                else if (name == "outline-right-color")
                    attr.GetTextBoxAttr().GetOutline().GetRight().SetColour(ColourStringToLong(value));
                else if (name == "outline-top-color")
                    attr.GetTextBoxAttr().GetOutline().GetTop().SetColour(ColourStringToLong(value));
                else if (name == "outline-bottom-color")
                    attr.GetTextBoxAttr().GetOutline().GetBottom().SetColour(ColourStringToLong(value));

                else if (name == "outline-left-width")
                    attr.GetTextBoxAttr().GetOutline().GetLeft().SetWidth(ParseDimension(value));
                else if (name == "outline-right-width")
                    attr.GetTextBoxAttr().GetOutline().GetRight().SetWidth(ParseDimension(value));
                else if (name == "outline-top-width")
                    attr.GetTextBoxAttr().GetOutline().GetTop().SetWidth(ParseDimension(value));
                else if (name == "outline-bottom-width")
                    attr.GetTextBoxAttr().GetOutline().GetBottom().SetWidth(ParseDimension(value));
            }
            else if (name.Contains("margin-"))
            {
                if (name == "margin-left")
                    attr.GetTextBoxAttr().GetMargins().GetLeft().SetValue(ParseDimension(value));
                else if (name == "margin-right")
                    attr.GetTextBoxAttr().GetMargins().GetRight().SetValue(ParseDimension(value));
                else if (name == "margin-top")
                    attr.GetTextBoxAttr().GetMargins().GetTop().SetValue(ParseDimension(value));
                else if (name == "margin-bottom")
                    attr.GetTextBoxAttr().GetMargins().GetBottom().SetValue(ParseDimension(value));
            }
            else if (name.Contains("padding-"))
            {
                if (name == "padding-left")
                    attr.GetTextBoxAttr().GetPadding().GetLeft().SetValue(ParseDimension(value));
                else if (name == "padding-right")
                    attr.GetTextBoxAttr().GetPadding().GetRight().SetValue(ParseDimension(value));
                else if (name == "padding-top")
                    attr.GetTextBoxAttr().GetPadding().GetTop().SetValue(ParseDimension(value));
                else if (name == "padding-bottom")
                    attr.GetTextBoxAttr().GetPadding().GetBottom().SetValue(ParseDimension(value));
            }
            else if (name.Contains("position-"))
            {
                if (name == "position-left")
                    attr.GetTextBoxAttr().GetPosition().GetLeft().SetValue(ParseDimension(value));
                else if (name == "position-right")
                    attr.GetTextBoxAttr().GetPosition().GetRight().SetValue(ParseDimension(value));
                else if (name == "position-top")
                    attr.GetTextBoxAttr().GetPosition().GetTop().SetValue(ParseDimension(value));
                else if (name == "position-bottom")
                    attr.GetTextBoxAttr().GetPosition().GetBottom().SetValue(ParseDimension(value));
            }
        }

        xmlAttr = xmlAttr->GetNext();
    }

    return true;
}

bool wxRichTextXMLHelper::ImportStyleDefinition(wxRichTextStyleSheet* sheet, wxXmlNode* node)
{
    wxString styleType = node->GetName();
    wxString styleName = node->GetAttribute("name", "");
    wxString baseStyleName = node->GetAttribute("basestyle", "");

    if (styleName.empty())
        return false;

    if (styleType == "characterstyle")
    {
        wxRichTextCharacterStyleDefinition* def = new wxRichTextCharacterStyleDefinition(styleName);
        def->SetBaseStyle(baseStyleName);

        wxXmlNode* child = node->GetChildren();
        while (child)
        {
            if (child->GetName() == "style")
            {
                wxRichTextAttr attr;
                ImportStyle(attr, child, false);
                def->SetStyle(attr);
            }
            child = child->GetNext();
        }

        ImportProperties(def->GetProperties(), node);

        sheet->AddCharacterStyle(def);
    }
    else if (styleType == "paragraphstyle")
    {
        wxRichTextParagraphStyleDefinition* def = new wxRichTextParagraphStyleDefinition(styleName);

        wxString nextStyleName = node->GetAttribute("nextstyle", "");
        def->SetNextStyle(nextStyleName);
        def->SetBaseStyle(baseStyleName);

        wxXmlNode* child = node->GetChildren();
        while (child)
        {
            if (child->GetName() == "style")
            {
                wxRichTextAttr attr;
                ImportStyle(attr, child, true);
                def->SetStyle(attr);
            }
            child = child->GetNext();
        }

        ImportProperties(def->GetProperties(), node);

        sheet->AddParagraphStyle(def);
    }
    else if (styleType == "boxstyle")
    {
        wxRichTextBoxStyleDefinition* def = new wxRichTextBoxStyleDefinition(styleName);

        def->SetBaseStyle(baseStyleName);

        wxXmlNode* child = node->GetChildren();
        while (child)
        {
            if (child->GetName() == "style")
            {
                wxRichTextAttr attr;
                ImportStyle(attr, child, true);
                def->SetStyle(attr);
            }
            child = child->GetNext();
        }

        ImportProperties(def->GetProperties(), node);

        sheet->AddBoxStyle(def);
    }
    else if (styleType == "liststyle")
    {
        wxRichTextListStyleDefinition* def = new wxRichTextListStyleDefinition(styleName);

        wxString nextStyleName = node->GetAttribute("nextstyle", "");
        def->SetNextStyle(nextStyleName);
        def->SetBaseStyle(baseStyleName);

        wxXmlNode* child = node->GetChildren();
        while (child)
        {
            if (child->GetName() == "style")
            {
                wxRichTextAttr attr;
                ImportStyle(attr, child, true);

                wxString styleLevel = child->GetAttribute("level", "");
                if (styleLevel.empty())
                {
                    def->SetStyle(attr);
                }
                else
                {
                    int level = wxAtoi(styleLevel);
                    if (level > 0 && level <= 10)
                    {
                        def->SetLevelAttributes(level-1, attr);
                    }
                }
            }
            child = child->GetNext();
        }

        ImportProperties(def->GetProperties(), node);

        sheet->AddListStyle(def);
    }

    return true;
}

bool wxRichTextXMLHelper::ImportProperties(wxRichTextProperties& properties, wxXmlNode* node)
{
    wxXmlNode* child = node->GetChildren();
    while (child)
    {
        if (child->GetName() == "properties")
        {
            wxXmlNode* propertyChild = child->GetChildren();
            while (propertyChild)
            {
                if (propertyChild->GetName() == "property")
                {
                    wxString name = propertyChild->GetAttribute("name", "");
                    wxString value = propertyChild->GetAttribute("value", "");
                    wxString type = propertyChild->GetAttribute("type", "");

                    wxVariant var = MakePropertyFromString(name, value, type);
                    if (!var.IsNull())
                    {
                        properties.SetProperty(var);
                    }
                }
                propertyChild = propertyChild->GetNext();
            }
        }
        child = child->GetNext();
    }
    return true;
}

#if wxRICHTEXT_HAVE_DIRECT_OUTPUT
// write string to output
void wxRichTextXMLHelper::OutputString(wxOutputStream& stream, const wxString& str,
                                [[maybe_unused]] wxMBConv* convMem, wxMBConv *convFile)
{
    if (str.empty()) return;

    if (convFile)
    {
        const wxWX2MBbuf buf(str.mb_str(*convFile));
        stream.Write((const char*)buf, strlen((const char*)buf));
    }
    else
    {
        const wxWX2MBbuf buf(str.mb_str(wxConvUTF8));
        stream.Write((const char*)buf, strlen((const char*)buf));
    }
}

void wxRichTextXMLHelper::OutputIndentation(wxOutputStream& stream, int indent)
{
    wxString str = "\n";
    for (int i = 0; i < indent; i++)
        str << wxT(' ') << wxT(' ');
    OutputString(stream, str, nullptr, nullptr);
}

// Same as above, but create entities first.
// Translates '<' to "&lt;", '>' to "&gt;" and '&' to "&amp;"
void wxRichTextXMLHelper::OutputStringEnt(wxOutputStream& stream, const wxString& str,
                            wxMBConv *convMem, wxMBConv *convFile)
{
    wxString buf;
    size_t i, last, len;
    wxChar c;

    len = str.Len();
    last = 0;
    for (i = 0; i < len; i++)
    {
        c = str.GetChar(i);

        // Original code excluded "&amp;" but we _do_ want to convert
        // the ampersand beginning &amp; because otherwise when read in,
        // the original "&amp;" becomes "&".

        if (c == wxT('<') || c == wxT('>') || c == wxT('"') ||
            (c == wxT('&') /* && (str.Mid(i+1, 4) != "amp;") */ ))
        {
            OutputString(stream, str.Mid(last, i - last), convMem, convFile);
            switch (c)
            {
            case wxT('<'):
                OutputString(stream, "&lt;", nullptr, nullptr);
                break;
            case wxT('>'):
                OutputString(stream, "&gt;", nullptr, nullptr);
                break;
            case wxT('&'):
                OutputString(stream, "&amp;", nullptr, nullptr);
                break;
            case wxT('"'):
                OutputString(stream, "&quot;", nullptr, nullptr);
                break;
            default: break;
            }
            last = i + 1;
        }
        else if (wxUChar(c) > 127)
        {
            OutputString(stream, str.Mid(last, i - last), convMem, convFile);

            wxString s("&#");
            s << (int) c;
            s << ";";
            OutputString(stream, s, nullptr, nullptr);
            last = i + 1;
        }
    }
    OutputString(stream, str.Mid(last, i - last), convMem, convFile);
}

void wxRichTextXMLHelper::OutputString(wxOutputStream& stream, const wxString& str)
{
    OutputString(stream, str, m_convMem, m_convFile);
}

void wxRichTextXMLHelper::OutputStringEnt(wxOutputStream& stream, const wxString& str)
{
    OutputStringEnt(stream, str, m_convMem, m_convFile);
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& name, const int& v)
{
    str << " " << name << wxT("=\"") << wxString::Format(wxT("%d"), v) << "\"";
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& name, const long& v)
{
    str << " " << name << wxT("=\"") << wxString::Format(wxT("%ld"), v) << "\"";
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& name, const double& v)
{
    str << " " << name << wxS("=\"") << wxString::Format(wxS("%.2f"), v) << "\"";
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& name, const wxChar* s)
{
    str << " " << name << wxT("=\"") << s << "\"";
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& name, const wxString& s)
{
    str << " " << name << wxT("=\"") << s << "\"";
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& name, const wxColour& col)
{
    str << " " << name << wxT("=\"") << "#" << ColourToHexString(col) << "\"";
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& name, const wxTextAttrDimension& dim)
{
    if (dim.IsValid())
    {
        wxString value = MakeString(dim.GetValue()) + "," + MakeString((int) dim.GetFlags());
        str << " " << name << "=\"";
        str << value;
        str << "\"";
    }
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& rootName, const wxTextAttrDimensions& dims)
{
    if (dims.GetLeft().IsValid())
        AddAttribute(str, rootName + wxString("-left"), dims.GetLeft());
    if (dims.GetRight().IsValid())
        AddAttribute(str, rootName + wxString("-right"), dims.GetRight());
    if (dims.GetTop().IsValid())
        AddAttribute(str, rootName + wxString("-top"), dims.GetTop());
    if (dims.GetBottom().IsValid())
        AddAttribute(str, rootName + wxString("-bottom"), dims.GetBottom());
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& rootName, const wxTextAttrBorder& border)
{
    if (border.HasStyle())
        AddAttribute(str, rootName + wxString("-style"), border.GetStyle());
    if (border.HasColour())
        AddAttribute(str, rootName + wxString("-color"), border.GetColour());
    if (border.HasWidth())
        AddAttribute(str, rootName + wxString("-width"), border.GetWidth());
}

void wxRichTextXMLHelper::AddAttribute(wxString& str, const wxString& rootName, const wxTextAttrBorders& borders)
{
    AddAttribute(str, rootName + wxString("-left"), borders.GetLeft());
    AddAttribute(str, rootName + wxString("-right"), borders.GetRight());
    AddAttribute(str, rootName + wxString("-top"), borders.GetTop());
    AddAttribute(str, rootName + wxString("-bottom"), borders.GetBottom());
}

/// Create a string containing style attributes
wxString wxRichTextXMLHelper::AddAttributes(const wxRichTextAttr& attr, bool isPara)
{
    wxString str;
    if (attr.HasTextColour() && attr.GetTextColour().IsOk())
        AddAttribute(str, "textcolor", attr.GetTextColour());

    if (attr.HasBackgroundColour() && attr.GetBackgroundColour().IsOk())
        AddAttribute(str, "bgcolor", attr.GetBackgroundColour());

    if (attr.HasFontPointSize())
        AddAttribute(str, "fontpointsize", attr.GetFontSize());
    else if (attr.HasFontPixelSize())
        AddAttribute(str, "fontpixelsize", attr.GetFontSize());

    if (attr.HasFontFamily())
        AddAttribute(str, "fontfamily", static_cast<int>(attr.GetFontFamily())); // FIXME: Stupid solution.

    if (attr.HasFontItalic())
        AddAttribute(str, "fontstyle", static_cast<int>(attr.GetFontStyle())); // FIXME: Stupid solution.

    if (attr.HasFontWeight())
        AddAttribute(str, "fontweight", attr.GetFontWeight());

    if (attr.HasFontUnderlined())
        AddAttribute(str, "fontunderlined", (int) attr.GetFontUnderlined());

    if (attr.HasFontFaceName())
        AddAttribute(str, "fontface", AttributeToXML(attr.GetFontFaceName()));

    if (attr.HasTextEffects())
    {
        AddAttribute(str, "texteffects", attr.GetTextEffects());
        AddAttribute(str, "texteffectflags", attr.GetTextEffectFlags());
    }

    if (!attr.GetCharacterStyleName().empty())
        AddAttribute(str, "characterstyle", AttributeToXML(attr.GetCharacterStyleName()));

    if (attr.HasURL())
        AddAttribute(str, "url", AttributeToXML(attr.GetURL()));

    if (isPara)
    {
        if (attr.HasAlignment())
            AddAttribute(str, "alignment", (int) attr.GetAlignment());

        if (attr.HasLeftIndent())
        {
            AddAttribute(str, "leftindent", (int) attr.GetLeftIndent());
            AddAttribute(str, "leftsubindent", (int) attr.GetLeftSubIndent());
        }

        if (attr.HasRightIndent())
            AddAttribute(str, "rightindent", (int) attr.GetRightIndent());

        if (attr.HasParagraphSpacingAfter())
            AddAttribute(str, "parspacingafter", (int) attr.GetParagraphSpacingAfter());

        if (attr.HasParagraphSpacingBefore())
            AddAttribute(str, "parspacingbefore", (int) attr.GetParagraphSpacingBefore());

        if (attr.HasLineSpacing())
            AddAttribute(str, "linespacing", (int) attr.GetLineSpacing());

        if (attr.HasBulletStyle())
            AddAttribute(str, "bulletstyle", (int) attr.GetBulletStyle());

        if (attr.HasBulletNumber())
            AddAttribute(str, "bulletnumber", (int) attr.GetBulletNumber());

        if (attr.HasBulletText())
        {
            // If using a bullet symbol, convert to integer in case it's a non-XML-friendly character.
            // Otherwise, assume it's XML-friendly text such as outline numbering, e.g. 1.2.3.1
            if (!attr.GetBulletText().empty() && (attr.GetBulletStyle() & wxTEXT_ATTR_BULLET_STYLE_SYMBOL))
                AddAttribute(str, "bulletsymbol", (int) (attr.GetBulletText()[0]));
            else
                AddAttribute(str, "bullettext", AttributeToXML(attr.GetBulletText()));

            AddAttribute(str, "bulletfont", attr.GetBulletFont());
        }

        if (attr.HasBulletName())
            AddAttribute(str, "bulletname", AttributeToXML(attr.GetBulletName()));

        if (!attr.GetParagraphStyleName().empty())
            AddAttribute(str, "parstyle", AttributeToXML(attr.GetParagraphStyleName()));

        if (!attr.GetListStyleName().empty())
            AddAttribute(str, "liststyle", AttributeToXML(attr.GetListStyleName()));

        if (!attr.GetTextBoxAttr().GetBoxStyleName().empty())
            AddAttribute(str, "boxstyle", AttributeToXML(attr.GetTextBoxAttr().GetBoxStyleName()));

        if (attr.HasTabs())
        {
            wxString strTabs;
            size_t i;
            for (i = 0; i < attr.GetTabs().size(); i++)
            {
                if (i > 0) strTabs << ",";
                strTabs << attr.GetTabs()[i];
            }
            AddAttribute(str, "tabs", strTabs);
        }

        if (attr.HasPageBreak())
        {
            AddAttribute(str, "pagebreak", 1);
        }

        if (attr.HasOutlineLevel())
            AddAttribute(str, "outlinelevel", (int) attr.GetOutlineLevel());
    }

    AddAttribute(str, "margin", attr.GetTextBoxAttr().GetMargins());
    AddAttribute(str, "padding", attr.GetTextBoxAttr().GetPadding());
    AddAttribute(str, "position", attr.GetTextBoxAttr().GetPosition());
    AddAttribute(str, "border", attr.GetTextBoxAttr().GetBorder());
    AddAttribute(str, "outline", attr.GetTextBoxAttr().GetOutline());
    AddAttribute(str, "width", attr.GetTextBoxAttr().GetWidth());
    AddAttribute(str, "height", attr.GetTextBoxAttr().GetHeight());
    AddAttribute(str, "minwidth", attr.GetTextBoxAttr().GetMinSize().GetWidth());
    AddAttribute(str, "minheight", attr.GetTextBoxAttr().GetMinSize().GetHeight());
    AddAttribute(str, "maxwidth", attr.GetTextBoxAttr().GetMaxSize().GetWidth());
    AddAttribute(str, "maxheight", attr.GetTextBoxAttr().GetMaxSize().GetHeight());
    AddAttribute(str, "corner-radius", attr.GetTextBoxAttr().GetCornerRadius());

    if (attr.GetTextBoxAttr().HasVerticalAlignment())
    {
        wxString value;
        if (attr.GetTextBoxAttr().GetVerticalAlignment() == wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_TOP)
            value = "top";
        else if (attr.GetTextBoxAttr().GetVerticalAlignment() == wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_CENTRE)
            value = "centre";
        else if (attr.GetTextBoxAttr().GetVerticalAlignment() == wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_BOTTOM)
            value = "bottom";
        else
            value = "none";
        AddAttribute(str, "verticalalignment", value);
    }

    if (attr.GetTextBoxAttr().HasFloatMode())
    {
        wxString value;
        if (attr.GetTextBoxAttr().GetFloatMode() == wxTEXT_BOX_ATTR_FLOAT_LEFT)
            value = "left";
        else if (attr.GetTextBoxAttr().GetFloatMode() == wxTEXT_BOX_ATTR_FLOAT_RIGHT)
            value = "right";
        else
            value = "none";
        AddAttribute(str, "float", value);
    }

    if (attr.GetTextBoxAttr().HasClearMode())
    {
        wxString value;
        if (attr.GetTextBoxAttr().GetClearMode() == wxTEXT_BOX_ATTR_CLEAR_LEFT)
            value = "left";
        else if (attr.GetTextBoxAttr().GetClearMode() == wxTEXT_BOX_ATTR_CLEAR_RIGHT)
            value = "right";
        else if (attr.GetTextBoxAttr().GetClearMode() == wxTEXT_BOX_ATTR_CLEAR_BOTH)
            value = "both";
        else
            value = "none";
        AddAttribute(str, "clear", value);
    }

    if (attr.GetTextBoxAttr().HasCollapseBorders())
        AddAttribute(str, "collapse-borders", (int) attr.GetTextBoxAttr().GetCollapseBorders());

    if (attr.GetTextBoxAttr().HasWhitespaceMode())
        AddAttribute(str, "whitespace-mode", (int) attr.GetTextBoxAttr().GetWhitespaceMode());
    return str;
}

// Create a string containing style attributes, plus further object 'attributes' (shown, id)
wxString wxRichTextXMLHelper::AddAttributes(wxRichTextObject* obj, bool isPara)
{
    wxString style = AddAttributes(obj->GetAttributes(), isPara);
    if (!obj->IsShown())
        style << " show=\"0\"";
    return style;
}    

// Write the properties
bool wxRichTextXMLHelper::WriteProperties(wxOutputStream& stream, const wxRichTextProperties& properties, int level)
{
    if (properties.GetCount() > 0)
    {
        level ++;

        OutputIndentation(stream, level);
        OutputString(stream, "<properties>");

        level ++;

        size_t i;
        for (i = 0; i < properties.GetCount(); i++)
        {
            const wxVariant& var = properties[i];
            if (!var.IsNull())
            {
                const wxString& name = var.GetName();
                wxString value = MakeStringFromProperty(var);

                OutputIndentation(stream, level);
                OutputString(stream, "<property name=\"" + name +
                    "\" type=\"" + var.GetType() + "\" value=\"");
                OutputStringEnt(stream, value);
                OutputString(stream, "\"/>");
            }
        }

        level --;

        OutputIndentation(stream, level);
        OutputString(stream, "</properties>");

        level --;
    }

    return true;
}

bool wxRichTextXMLHelper::ExportStyleDefinition(wxOutputStream& stream, wxRichTextStyleDefinition* def, int level)
{
    wxRichTextCharacterStyleDefinition* charDef = wxDynamicCast(def, wxRichTextCharacterStyleDefinition);
    wxRichTextParagraphStyleDefinition* paraDef = wxDynamicCast(def, wxRichTextParagraphStyleDefinition);
    wxRichTextListStyleDefinition* listDef = wxDynamicCast(def, wxRichTextListStyleDefinition);
    wxRichTextBoxStyleDefinition* boxDef = wxDynamicCast(def, wxRichTextBoxStyleDefinition);

    wxString name = def->GetName();
    wxString nameProp;
    if (!name.empty())
        nameProp = " name=\"" + AttributeToXML(name) + "\"";

    wxString baseStyle = def->GetBaseStyle();
    wxString baseStyleProp;
    if (!baseStyle.empty())
        baseStyleProp = " basestyle=\"" + AttributeToXML(baseStyle) + "\"";

    wxString descr = def->GetDescription();
    wxString descrProp;
    if (!descr.empty())
        descrProp = " description=\"" + AttributeToXML(descr) + "\"";

    if (charDef)
    {
        OutputIndentation(stream, level);
        OutputString(stream, "<characterstyle" + nameProp + baseStyleProp + descrProp + ">");

        level ++;

        wxString style = AddAttributes(def->GetStyle(), false);

        OutputIndentation(stream, level);
        OutputString(stream, "<style " + style + ">");

        OutputIndentation(stream, level);
        OutputString(stream, "</style>");

        level --;

        OutputIndentation(stream, level);
        OutputString(stream, "</characterstyle>");
    }
    else if (listDef)
    {
        OutputIndentation(stream, level);

        if (!listDef->GetNextStyle().empty())
            baseStyleProp << " nextstyle=\"" << AttributeToXML(listDef->GetNextStyle()) << "\"";

        OutputString(stream, "<liststyle" + nameProp + baseStyleProp + descrProp + ">");

        level ++;

        wxString style = AddAttributes(def->GetStyle(), true);

        OutputIndentation(stream, level);
        OutputString(stream, "<style " + style + ">");

        OutputIndentation(stream, level);
        OutputString(stream, "</style>");

        int i;
        for (i = 0; i < 10; i ++)
        {
            wxRichTextAttr* levelAttr = listDef->GetLevelAttributes(i);
            if (levelAttr)
            {
                wxString levelStyle = AddAttributes(def->GetStyle(), true);
                wxString levelStr = wxString::Format(" level=\"%d\" ", (i+1));

                OutputIndentation(stream, level);
                OutputString(stream, "<style " + levelStr + levelStyle + ">");

                OutputIndentation(stream, level);
                OutputString(stream, "</style>");
            }
        }

        level --;

        OutputIndentation(stream, level);
        OutputString(stream, "</liststyle>");
    }
    else if (paraDef)
    {
        OutputIndentation(stream, level);

        if (!paraDef->GetNextStyle().empty())
            baseStyleProp << " nextstyle=\"" << AttributeToXML(paraDef->GetNextStyle()) << "\"";

        OutputString(stream, "<paragraphstyle" + nameProp + baseStyleProp + descrProp + ">");

        level ++;

        wxString style = AddAttributes(def->GetStyle(), true);

        OutputIndentation(stream, level);
        OutputString(stream, "<style " + style + ">");

        OutputIndentation(stream, level);
        OutputString(stream, "</style>");

        level --;

        OutputIndentation(stream, level);
        OutputString(stream, "</paragraphstyle>");
    }
    else if (boxDef)
    {
        OutputIndentation(stream, level);

        OutputString(stream, "<boxstyle" + nameProp + baseStyleProp + descrProp + ">");

        level ++;

        wxString style = AddAttributes(def->GetStyle(), true);

        OutputIndentation(stream, level);
        OutputString(stream, "<style " + style + ">");

        OutputIndentation(stream, level);
        OutputString(stream, "</style>");

        level --;

        OutputIndentation(stream, level);
        OutputString(stream, "</boxstyle>");
    }

    WriteProperties(stream, def->GetProperties(), level);

    return true;
}

#endif
    // wxRICHTEXT_HAVE_DIRECT_OUTPUT


#if wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& name, const int& v)
{
    node->AddAttribute(name, MakeString(v));
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& name, const long& v)
{
    node->AddAttribute(name, MakeString(v));
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& name, const double& v)
{
    node->AddAttribute(name, MakeString(v));
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& name, const wxString& s)
{
    node->AddAttribute(name, s);
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& name, const wxColour& col)
{
    node->AddAttribute(name, MakeString(col));
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& name, const wxTextAttrDimension& dim)
{
    if (dim.IsValid())
    {
        wxString value = MakeString(dim.GetValue()) + "," + MakeString(dim.GetFlags());
        AddAttribute(node, name, value);
    }
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& rootName, const wxTextAttrDimensions& dims)
{
    if (dims.GetLeft().IsValid())
        AddAttribute(node, rootName + wxString("-left"), dims.GetLeft());
    if (dims.GetRight().IsValid())
        AddAttribute(node, rootName + wxString("-right"), dims.GetRight());
    if (dims.GetTop().IsValid())
        AddAttribute(node, rootName + wxString("-top"), dims.GetTop());
    if (dims.GetBottom().IsValid())
        AddAttribute(node, rootName + wxString("-bottom"), dims.GetBottom());
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& rootName, const wxTextAttrBorder& border)
{
    if (border.HasStyle())
        AddAttribute(node, rootName + wxString("-style"), border.GetStyle());
    if (border.HasColour())
        AddAttribute(node, rootName + wxString("-color"), border.GetColour());
    if (border.HasWidth())
        AddAttribute(node, rootName + wxString("-width"), border.GetWidth());
}

void wxRichTextXMLHelper::AddAttribute(wxXmlNode* node, const wxString& rootName, const wxTextAttrBorders& borders)
{
    AddAttribute(node, rootName + wxString("-left"), borders.GetLeft());
    AddAttribute(node, rootName + wxString("-right"), borders.GetRight());
    AddAttribute(node, rootName + wxString("-top"), borders.GetTop());
    AddAttribute(node, rootName + wxString("-bottom"), borders.GetBottom());
}

bool wxRichTextXMLHelper::ExportStyleDefinition(wxXmlNode* parent, wxRichTextStyleDefinition* def)
{
    wxRichTextCharacterStyleDefinition* charDef = wxDynamicCast(def, wxRichTextCharacterStyleDefinition);
    wxRichTextParagraphStyleDefinition* paraDef = wxDynamicCast(def, wxRichTextParagraphStyleDefinition);
    wxRichTextBoxStyleDefinition* boxDef = wxDynamicCast(def, wxRichTextBoxStyleDefinition);
    wxRichTextListStyleDefinition* listDef = wxDynamicCast(def, wxRichTextListStyleDefinition);

    wxString baseStyle = def->GetBaseStyle();
    wxString descr = def->GetDescription();

    wxXmlNode* defNode = new wxXmlNode(wxXML_ELEMENT_NODE, {});
    parent->AddChild(defNode);
    if (!baseStyle.empty())
        defNode->AddAttribute("basestyle", baseStyle);
    if (!descr.empty())
        defNode->AddAttribute("description", descr);

    wxXmlNode* styleNode = new wxXmlNode(wxXML_ELEMENT_NODE, "style");
    defNode->AddChild(styleNode);

    if (charDef)
    {
        defNode->SetName("characterstyle");
        AddAttributes(styleNode, def->GetStyle(), false);
    }
    else if (listDef)
    {
        defNode->SetName("liststyle");

        if (!listDef->GetNextStyle().empty())
            defNode->AddAttribute("nextstyle", listDef->GetNextStyle());

        AddAttributes(styleNode, def->GetStyle(), true);

        int i;
        for (i = 0; i < 10; i ++)
        {
            wxRichTextAttr* levelAttr = listDef->GetLevelAttributes(i);
            if (levelAttr)
            {
                wxXmlNode* levelNode = new wxXmlNode(wxXML_ELEMENT_NODE, "style");
                defNode->AddChild(levelNode);
                levelNode->AddAttribute("level", MakeString(i+1));
                AddAttributes(levelNode, * levelAttr, true);
            }
        }
    }
    else if (boxDef)
    {
        defNode->SetName("boxstyle");

        AddAttributes(styleNode, def->GetStyle(), true);
    }
    else if (paraDef)
    {
        defNode->SetName("paragraphstyle");

        if (!paraDef->GetNextStyle().empty())
            defNode->AddAttribute("nextstyle", paraDef->GetNextStyle());

        AddAttributes(styleNode, def->GetStyle(), true);
    }

    WriteProperties(defNode, def->GetProperties());

    return true;
}

bool wxRichTextXMLHelper::AddAttributes(wxXmlNode* node, wxRichTextAttr& attr, bool isPara)
{
    if (attr.HasTextColour() && attr.GetTextColour().IsOk())
        node->AddAttribute("textcolor", MakeString(attr.GetTextColour()));
    if (attr.HasBackgroundColour() && attr.GetBackgroundColour().IsOk())
        node->AddAttribute("bgcolor", MakeString(attr.GetBackgroundColour()));

    if (attr.HasFontPointSize())
        node->AddAttribute("fontpointsize", MakeString(attr.GetFontSize()));
    else if (attr.HasFontPixelSize())
        node->AddAttribute("fontpixelsize", MakeString(attr.GetFontSize()));
    if (attr.HasFontFamily())
        node->AddAttribute("fontfamily", MakeString(static_cast<int>(attr.GetFontFamily()))); // FIXME: Stupid solution.s
    if (attr.HasFontItalic())
        node->AddAttribute("fontstyle", MakeString(static_cast<int>(attr.GetFontStyle()))); // FIXME: Stupid solution.
    if (attr.HasFontWeight())
        node->AddAttribute("fontweight", MakeString(attr.GetFontWeight()));
    if (attr.HasFontUnderlined())
        node->AddAttribute("fontunderlined", MakeString((int) attr.GetFontUnderlined()));
    if (attr.HasFontFaceName())
        node->AddAttribute("fontface", attr.GetFontFaceName());

    if (attr.HasTextEffects())
    {
        node->AddAttribute("texteffects", MakeString(attr.GetTextEffects()));
        node->AddAttribute("texteffectflags", MakeString(attr.GetTextEffectFlags()));
    }
    if (attr.HasCharacterStyleName() && !attr.GetCharacterStyleName().empty())
        node->AddAttribute("characterstyle", attr.GetCharacterStyleName());

    if (attr.HasURL())
        node->AddAttribute("url", attr.GetURL()); // TODO: do we need to wrap this in AttributeToXML?

    if (isPara)
    {
        if (attr.HasAlignment())
            node->AddAttribute("alignment", MakeString((int) attr.GetAlignment()));

        if (attr.HasLeftIndent())
        {
            node->AddAttribute("leftindent", MakeString((int) attr.GetLeftIndent()));
            node->AddAttribute("leftsubindent", MakeString((int) attr.GetLeftSubIndent()));
        }

        if (attr.HasRightIndent())
            node->AddAttribute("rightindent", MakeString((int) attr.GetRightIndent()));

        if (attr.HasParagraphSpacingAfter())
            node->AddAttribute("parspacingafter", MakeString((int) attr.GetParagraphSpacingAfter()));

        if (attr.HasParagraphSpacingBefore())
            node->AddAttribute("parspacingbefore", MakeString((int) attr.GetParagraphSpacingBefore()));

        if (attr.HasLineSpacing())
            node->AddAttribute("linespacing", MakeString((int) attr.GetLineSpacing()));

        if (attr.HasBulletStyle())
            node->AddAttribute("bulletstyle", MakeString((int) attr.GetBulletStyle()));

        if (attr.HasBulletNumber())
            node->AddAttribute("bulletnumber", MakeString((int) attr.GetBulletNumber()));

        if (attr.HasBulletText())
        {
            // If using a bullet symbol, convert to integer in case it's a non-XML-friendly character.
            // Otherwise, assume it's XML-friendly text such as outline numbering, e.g. 1.2.3.1
            if (!attr.GetBulletText().empty() && (attr.GetBulletStyle() & wxTEXT_ATTR_BULLET_STYLE_SYMBOL))
                node->AddAttribute("bulletsymbol", MakeString((int) (attr.GetBulletText()[0])));
            else
                node->AddAttribute("bullettext", attr.GetBulletText());

            if (!attr.GetBulletFont().empty())
                node->AddAttribute("bulletfont", attr.GetBulletFont());
        }

        if (attr.HasBulletName())
            node->AddAttribute("bulletname", attr.GetBulletName());

        if (!attr.GetParagraphStyleName().empty())
            node->AddAttribute("parstyle", attr.GetParagraphStyleName());

        if (!attr.GetListStyleName().empty())
            node->AddAttribute("liststyle", attr.GetListStyleName());

        if (!attr.GetTextBoxAttr().GetBoxStyleName().empty())
            node->AddAttribute("boxstyle", attr.GetTextBoxAttr().GetBoxStyleName());

        if (attr.HasTabs())
        {
            wxString tabs;
            size_t i;
            for (i = 0; i < attr.GetTabs().size(); i++)
            {
                if (i > 0)
                    tabs << ",";
                tabs << attr.GetTabs()[i];
            }
            node->AddAttribute("tabs", tabs);
        }

        if (attr.HasPageBreak())
            node->AddAttribute("pagebreak", "1");

        if (attr.HasOutlineLevel())
            node->AddAttribute("outlinelevel", MakeString((int) attr.GetOutlineLevel()));
    }

    AddAttribute(node, "margin", attr.GetTextBoxAttr().GetMargins());
    AddAttribute(node, "padding", attr.GetTextBoxAttr().GetPadding());
    AddAttribute(node, "position", attr.GetTextBoxAttr().GetPosition());
    AddAttribute(node, "border", attr.GetTextBoxAttr().GetBorder());
    AddAttribute(node, "outline", attr.GetTextBoxAttr().GetOutline());
    AddAttribute(node, "width", attr.GetTextBoxAttr().GetWidth());
    AddAttribute(node, "height", attr.GetTextBoxAttr().GetHeight());
    AddAttribute(node, "minwidth", attr.GetTextBoxAttr().GetMinSize().GetWidth());
    AddAttribute(node, "minheight", attr.GetTextBoxAttr().GetMinSize().GetHeight());
    AddAttribute(node, "maxwidth", attr.GetTextBoxAttr().GetMaxSize().GetWidth());
    AddAttribute(node, "maxheight", attr.GetTextBoxAttr().GetMaxSize().GetHeight());
    AddAttribute(node, "corner-radius", attr.GetTextBoxAttr().GetCornerRadius());

    if (attr.GetTextBoxAttr().HasVerticalAlignment())
    {
        wxString value;
        if (attr.GetTextBoxAttr().GetVerticalAlignment() == wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_TOP)
            value = "top";
        else if (attr.GetTextBoxAttr().GetVerticalAlignment() == wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_CENTRE)
            value = "centre";
        else if (attr.GetTextBoxAttr().GetVerticalAlignment() == wxTEXT_BOX_ATTR_VERTICAL_ALIGNMENT_BOTTOM)
            value = "bottom";
        else
            value = "none";
        AddAttribute(node, "verticalalignment", value);
    }

    if (attr.GetTextBoxAttr().HasFloatMode())
    {
        wxString value;
        if (attr.GetTextBoxAttr().GetFloatMode() == wxTEXT_BOX_ATTR_FLOAT_LEFT)
            value = "left";
        else if (attr.GetTextBoxAttr().GetFloatMode() == wxTEXT_BOX_ATTR_FLOAT_RIGHT)
            value = "right";
        else
            value = "none";
        AddAttribute(node, "float", value);
    }

    if (attr.GetTextBoxAttr().HasClearMode())
    {
        wxString value;
        if (attr.GetTextBoxAttr().GetClearMode() == wxTEXT_BOX_ATTR_CLEAR_LEFT)
            value = "left";
        else if (attr.GetTextBoxAttr().GetClearMode() == wxTEXT_BOX_ATTR_CLEAR_RIGHT)
            value = "right";
        else if (attr.GetTextBoxAttr().GetClearMode() == wxTEXT_BOX_ATTR_CLEAR_BOTH)
            value = "both";
        else
            value = "none";
        AddAttribute(node, "clear", value);
    }

    if (attr.GetTextBoxAttr().HasCollapseBorders())
        AddAttribute(node, "collapse-borders", (int) attr.GetTextBoxAttr().GetCollapseBorders());

    if (attr.GetTextBoxAttr().HasWhitespaceMode())
        AddAttribute(node, "whitespace-mode", (int) attr.GetTextBoxAttr().GetWhitespaceMode());

    return true;
}

bool wxRichTextXMLHelper::AddAttributes(wxXmlNode* node, wxRichTextObject* obj, bool isPara)
{
    if (obj)
    {
        if (!obj->IsShown())
            node->AddAttribute("show", "0");
    }

    return AddAttributes(node, obj->GetAttributes(), isPara);
}

bool wxRichTextXMLHelper::WriteProperties(wxXmlNode* node, const wxRichTextProperties& properties)
{
    if (properties.GetCount() > 0)
    {
        wxXmlNode* propertiesNode = new wxXmlNode(wxXML_ELEMENT_NODE, "properties");
        node->AddChild(propertiesNode);
        size_t i;
        for (i = 0; i < properties.GetCount(); i++)
        {
            const wxVariant& var = properties[i];
            if (!var.IsNull())
            {
                wxXmlNode* propertyNode = new wxXmlNode(wxXML_ELEMENT_NODE, "property");
                propertiesNode->AddChild(propertyNode);

                const wxString& name = var.GetName();
                wxString value = MakeStringFromProperty(var);

                AddAttribute(propertyNode, "name", name);
                AddAttribute(propertyNode, "type", var.GetType());
                AddAttribute(propertyNode, "value", value);
            }
        }
    }
    return true;
}

#endif
    // wxRICHTEXT_HAVE_XMLDOCUMENT_OUTPUT

#endif
    // wxUSE_RICHTEXT && wxUSE_XML

