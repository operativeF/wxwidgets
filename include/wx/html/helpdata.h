/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/helpdata.h
// Purpose:     wxHtmlHelpData
// Notes:       Based on htmlhelp.cpp, implementing a monolithic
//              HTML Help controller class,  by Vaclav Slavik
// Author:      Harm van der Heijden and Vaclav Slavik
// Copyright:   (c) Harm van der Heijden and Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPDATA_H_
#define _WX_HELPDATA_H_

#if wxUSE_HTML

#include "wx/object.h"
#include "wx/string.h"
#include "wx/filesys.h"
#include "wx/font.h"

import WX.Cmn.Stream;

class wxHtmlHelpData;

//--------------------------------------------------------------------------------
// helper classes & structs
//--------------------------------------------------------------------------------

class wxHtmlBookRecord
{
public:
    wxHtmlBookRecord(const wxString& bookfile, const wxString& basepath,
                     const wxString& title, const wxString& start)
        : m_BookFile(bookfile)
        , m_BasePath(basepath)
        , m_Title(title)
        , m_Start(start)
    {
        // for debugging, give the contents index obvious default values
        m_ContentsStart = m_ContentsEnd = -1;
    }
    wxString GetBookFile() const { return m_BookFile; }
    wxString GetTitle() const { return m_Title; }
    wxString GetStart() const { return m_Start; }
    wxString GetBasePath() const { return m_BasePath; }
    /* SetContentsRange: store in the bookrecord where in the index/contents lists the
     * book's records are stored. This to facilitate searching in a specific book.
     * This code will have to be revised when loading/removing books becomes dynamic.
     * (as opposed to appending only)
     * Note that storing index range is pointless, because the index is alphab. sorted. */
    void SetContentsRange(int start, int end) { m_ContentsStart = start; m_ContentsEnd = end; }
    int GetContentsStart() const { return m_ContentsStart; }
    int GetContentsEnd() const { return m_ContentsEnd; }

    void SetTitle(const wxString& title) { m_Title = title; }
    void SetBasePath(const wxString& path) { m_BasePath = path; }
    void SetStart(const wxString& start) { m_Start = start; }

    // returns full filename of page (which is part of the book),
    // i.e. with book's basePath prepended. If page is already absolute
    // path, basePath is _not_ prepended.
    wxString GetFullPath(const wxString &page) const;

protected:
    wxString m_BookFile;
    wxString m_BasePath;
    wxString m_Title;
    wxString m_Start;
    int m_ContentsStart;
    int m_ContentsEnd;
};


WX_DECLARE_OBJARRAY(wxHtmlBookRecord, wxHtmlBookRecArray);

struct wxHtmlHelpDataItem
{
    wxHtmlHelpDataItem()  = default;

    int level{0};
    wxHtmlHelpDataItem *parent{nullptr};
    int id{wxID_ANY};
    wxString name;
    std::string page;
    wxHtmlBookRecord *book{nullptr};

    // returns full filename of m_Page, i.e. with book's basePath prepended
    wxString GetFullPath() const { return book->GetFullPath(page); }

    // returns item indented with spaces if it has level>1:
    wxString GetIndentedName() const;
};

WX_DECLARE_OBJARRAY(wxHtmlHelpDataItem, wxHtmlHelpDataItems);


//------------------------------------------------------------------------------
// wxHtmlSearchEngine
//                  This class takes input streams and scans them for occurrence
//                  of keyword(s)
//------------------------------------------------------------------------------

class wxHtmlSearchEngine
{
public:
    wxHtmlSearchEngine()  = default;
    virtual ~wxHtmlSearchEngine() = default;

    wxHtmlSearchEngine(const wxHtmlSearchEngine&) = delete;
	wxHtmlSearchEngine& operator=(const wxHtmlSearchEngine&) = delete;

    // Sets the keyword we will be searching for
    virtual void LookFor(const wxString& keyword, bool case_sensitive, bool whole_words_only);

    // Scans the stream for the keyword.
    // Returns true if the stream contains keyword, fALSE otherwise
    virtual bool Scan(const wxFSFile& file);

private:
    wxString m_Keyword;
    bool m_CaseSensitive;
    bool m_WholeWords;
};


// State information of a search action. I'd have preferred to make this a
// nested class inside wxHtmlHelpData, but that's against coding standards :-(
// Never construct this class yourself, obtain a copy from
// wxHtmlHelpData::PrepareKeywordSearch(const wxString& key)
class wxHtmlSearchStatus
{
public:
    // constructor; supply wxHtmlHelpData ptr, the keyword and (optionally) the
    // title of the book to search. By default, all books are searched.
    wxHtmlSearchStatus(wxHtmlHelpData* base, const wxString& keyword,
                       bool case_sensitive, bool whole_words_only,
                       const wxString& book = {});

    wxHtmlSearchStatus(const wxHtmlSearchStatus&) = delete;
	wxHtmlSearchStatus& operator=(const wxHtmlSearchStatus&) = delete;

    bool Search();  // do the next iteration
    bool IsActive() const { return m_Active; }
    int GetCurIndex() const { return m_CurIndex; }
    int GetMaxIndex() const { return m_MaxIndex; }
    const wxString& GetName() const { return m_Name; }

    const wxHtmlHelpDataItem *GetCurItem() const { return m_CurItem; }

private:
    wxHtmlHelpData* m_Data;
    wxHtmlSearchEngine m_Engine;
    wxString m_Keyword, m_Name;
    wxString m_LastPage;
    wxHtmlHelpDataItem* m_CurItem;
    bool m_Active;   // search is not finished
    int m_CurIndex;  // where we are now
    int m_MaxIndex;  // number of files we search
    // For progress bar: 100*curindex/maxindex = % complete
};

class wxHtmlHelpData : public wxObject
{
    wxDECLARE_DYNAMIC_CLASS(wxHtmlHelpData);
    friend class wxHtmlSearchStatus;

public:
    wxHtmlHelpData() = default;

    wxHtmlHelpData(const wxHtmlHelpData&) = delete;
	wxHtmlHelpData& operator=(const wxHtmlHelpData&) = delete;

    // Sets directory where temporary files are stored.
    // These temp files are index & contents file in binary (much faster to read)
    // form. These files are NOT deleted on program's exit.
    void SetTempDir(const wxString& path);

    // Adds new book. 'book' is location of .htb file (stands for "html book").
    // See documentation for details on its format.
    // Returns success.
    bool AddBook(const wxString& book);
    bool AddBookParam(const wxFSFile& bookfile,
                      wxFontEncoding encoding,
                      const wxString& title, const wxString& contfile,
                      const wxString& indexfile = {},
                      const wxString& deftopic = {},
                      const wxString& path = {});

    // Some accessing stuff:

    // returns URL of page on basis of (file)name
    wxString FindPageByName(const wxString& page);
    // returns URL of page on basis of MS id
    wxString FindPageById(int id);

    const wxHtmlBookRecArray& GetBookRecArray() const { return m_bookRecords; }

    const wxHtmlHelpDataItems& GetContentsArray() const { return m_contents; }
    const wxHtmlHelpDataItems& GetIndexArray() const { return m_index; }

protected:
    wxString m_tempPath;

    // each book has one record in this array:
    wxHtmlBookRecArray m_bookRecords;

    wxHtmlHelpDataItems m_contents; // list of all available books and pages
    wxHtmlHelpDataItems m_index; // list of index items

protected:
    // Imports .hhp files (MS HTML Help Workshop)
    bool LoadMSProject(wxHtmlBookRecord *book, wxFileSystem& fsys,
                       const wxString& indexfile, const wxString& contentsfile);
    // Reads binary book
    bool LoadCachedBook(wxHtmlBookRecord *book, wxInputStream *f);
    // Writes binary book
    bool SaveCachedBook(wxHtmlBookRecord *book, wxOutputStream *f);
};

#endif

#endif
