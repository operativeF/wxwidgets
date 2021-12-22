/////////////////////////////////////////////////////////////////////////////
// Name:        src/html/htmlfilt.cpp
// Purpose:     wxHtmlFilter - input filter for translating into HTML format
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_HTML && wxUSE_STREAMS

#include "wx/log.h"
#include "wx/intl.h"

#include "wx/strconv.h"
#include "wx/html/htmlfilt.h"
#include "wx/html/htmlwin.h"

import WX.Cmn.StrStream;

// utility function: read entire contents of an wxInputStream into a wxString
//
// TODO: error handling?
static void ReadString(wxString& str, wxInputStream* s, wxMBConv& conv)
{
    wxStringOutputStream out(&str, conv);
    s->Read(out);
}

/*

There is code for several default filters:

*/

wxIMPLEMENT_ABSTRACT_CLASS(wxHtmlFilter, wxObject);

//--------------------------------------------------------------------------------
// wxHtmlFilterPlainText
//          filter for text/plain or uknown
//--------------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxHtmlFilterPlainText, wxHtmlFilter);

bool wxHtmlFilterPlainText::CanRead([[maybe_unused]] const wxFSFile& file) const
{
    return true;
}



wxString wxHtmlFilterPlainText::ReadFile(const wxFSFile& file) const
{
    wxInputStream *s = file.GetStream();
    wxString doc, doc2;

    if (s == nullptr) return {};
    ReadString(doc, s, wxConvISO8859_1);

    doc.Replace("&", "&amp;", true);
    doc.Replace("<", "&lt;", true);
    doc.Replace(">", "&gt;", true);
    doc2 = "<HTML><BODY><PRE>\n" + doc + "\n</PRE></BODY></HTML>";
    return doc2;
}





//--------------------------------------------------------------------------------
// wxHtmlFilterImage
//          filter for image/*
//--------------------------------------------------------------------------------

class wxHtmlFilterImage : public wxHtmlFilter
{
public:
    bool CanRead(const wxFSFile& file) const override;
    wxString ReadFile(const wxFSFile& file) const override;
};

bool wxHtmlFilterImage::CanRead(const wxFSFile& file) const
{
    return (file.GetMimeType().Left(6) == "image/");
}

wxString wxHtmlFilterImage::ReadFile(const wxFSFile& file) const
{
    wxString res = "<HTML><BODY><IMG SRC=\"" + file.GetLocation() + "\"></BODY></HTML>";
    return res;
}


//--------------------------------------------------------------------------------
// wxHtmlFilterHTML
//          filter for text/html
//--------------------------------------------------------------------------------


wxIMPLEMENT_DYNAMIC_CLASS(wxHtmlFilterHTML, wxHtmlFilter);

bool wxHtmlFilterHTML::CanRead(const wxFSFile& file) const
{
//    return (file.GetMimeType() == "text/html");
// This is true in most case but some page can return:
// "text/html; char-encoding=...."
// So we use Find instead
  return (file.GetMimeType().Find("text/html") == 0);
}



wxString wxHtmlFilterHTML::ReadFile(const wxFSFile& file) const
{
    wxInputStream *s = file.GetStream();
    wxString doc;

    if (s == nullptr)
    {
        wxLogError(_("Cannot open HTML document: %s"), file.GetLocation().c_str());
        return {};
    }

    // NB: We convert input file to wchar_t here in Unicode mode, based on
    //     either Content-Type header or <meta> tags. In ANSI mode, we don't
    //     do it as it is done by wxHtmlParser (for this reason, we add <meta>
    //     tag if we used Content-Type header).
    int charsetPos;
    if ((charsetPos = file.GetMimeType().Find("; charset=")) != wxNOT_FOUND)
    {
        wxString charset = file.GetMimeType().Mid(charsetPos + 10);
        wxCSConv conv(charset);
        ReadString(doc, s, conv);
    }
    else
    {
        size_t size = s->GetSize();
        wxCharBuffer buf( size );
        s->Read( buf.data(), size );
        wxString tmpdoc( buf, wxConvISO8859_1);

        wxString charset = wxHtmlParser::ExtractCharsetInformation(tmpdoc);
        if (charset.empty())
            doc = tmpdoc;
        else
        {
            wxCSConv conv(charset);
            doc = wxString( buf, conv );
        }
    }

    return doc;
}




///// Module:

class wxHtmlFilterModule : public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxHtmlFilterModule);

    public:
        bool OnInit() override
        {
            wxHtmlWindow::AddFilter(new wxHtmlFilterHTML);
            wxHtmlWindow::AddFilter(new wxHtmlFilterImage);
            return true;
        }
        void OnExit() override {}
};

wxIMPLEMENT_DYNAMIC_CLASS(wxHtmlFilterModule, wxModule);

#endif
