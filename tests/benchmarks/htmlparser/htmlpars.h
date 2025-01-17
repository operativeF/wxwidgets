/////////////////////////////////////////////////////////////////////////////
// Name:        htmlpars.h
// Purpose:     wx28HtmlParser class (generic parser)
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HTMLPARS_H_
#define _WX_HTMLPARS_H_

#include "wx/defs.h"
#include "wx/filesys.h"
#include "wx/hash.h"

#include "htmltag.h"

import WX.Base.FontEnc;

class  wxMBConv;
class  wx28HtmlParser;
class  wx28HtmlTagHandler;
class  wx28HtmlEntitiesParser;

class wx28HtmlTextPieces;
class wx28HtmlParserState;


enum wx28HtmlURLType
{
    wxHtmlURLType::Page,
    wxHtmlURLType::Image,
    wxHtmlURLType::Other
};

// This class handles generic parsing of HTML document : it scans
// the document and divides it into blocks of tags (where one block
// consists of starting and ending tag and of text between these
// 2 tags.
class  wx28HtmlParser : public wxObject
{
    wxDECLARE_ABSTRACT_CLASS(wx28HtmlParser);

public:
    wx28HtmlParser();
    virtual ~wx28HtmlParser();

    // Sets the class which will be used for opening files
    void SetFS(wxFileSystem *fs) { m_FS = fs; }

    wxFileSystem* GetFS() const { return m_FS; }

    // Opens file if the parser is allowed to open given URL (may be forbidden
    // for security reasons)
    virtual wxFSFile *OpenURL(wx28HtmlURLType type, const wxString& url) const;

    // You can simply call this method when you need parsed output.
    // This method does these things:
    // 1. call InitParser(source);
    // 2. call DoParsing();
    // 3. call GetProduct(); (its return value is then returned)
    // 4. call DoneParser();
    wxObject* Parse(const wxString& source);

    // Sets the source. This must be called before running Parse() method.
    virtual void InitParser(const wxString& source);
    // This must be called after Parse().
    virtual void DoneParser();

    // May be called during parsing to immediately return from Parse().
    virtual void StopParsing() { m_stopParsing = true; }

    // Parses the m_Source from begin_pos to end_pos-1.
    // (in noparams version it parses whole m_Source)
    void DoParsing(int begin_pos, int end_pos);
    void DoParsing();

    // Returns pointer to the tag at parser's current position
    wx28HtmlTag *GetCurrentTag() const { return m_CurTag; }

    // Returns product of parsing
    // Returned value is result of parsing of the part. The type of this result
    // depends on internal representation in derived parser
    // (see wx28HtmlWinParser for details).
    virtual wxObject* GetProduct() = 0;

    // adds handler to the list & hash table of handlers.
    virtual void AddTagHandler(wx28HtmlTagHandler *handler);

    // Forces the handler to handle additional tags (not returned by GetSupportedTags).
    // The handler should already be in use by this parser.
    // Example: you want to parse following pseudo-html structure:
    //   <myitems>
    //     <it name="one" value="1">
    //     <it name="two" value="2">
    //   </myitems>
    //   <it> This last it has different meaning, we don't want it to be parsed by myitems handler!
    // handler can handle only 'myitems' (e.g. its GetSupportedTags returns "MYITEMS")
    // you can call PushTagHandler(handler, "IT") when you find <myitems>
    // and call PopTagHandler() when you find </myitems>
    void PushTagHandler(wx28HtmlTagHandler *handler, const wxString& tags);

    // Restores state before last call to PushTagHandler
    void PopTagHandler();

    wxString* GetSource() {return &m_Source;}
    void SetSource(const wxString& src);

    // Sets HTML source and remembers current parser's state so that it can
    // later be restored. This is useful for on-line modifications of
    // HTML source (for example, <pre> handler replaces spaces with &nbsp;
    // and newlines with <br>)
    virtual void SetSourceAndSaveState(const wxString& src);
    // Restores parser's state from stack or returns false if the stack is
    // empty
    virtual bool RestoreState();

    // Returns HTML source inside the element (i.e. between the starting
    // and ending tag)
    wxString GetInnerSource(const wx28HtmlTag& tag);

    // Parses HTML string 'markup' and extracts charset info from <meta> tag
    // if present. Returns empty string if the tag is missing.
    // For wxHTML's internal use.
    static wxString ExtractCharsetInformation(const wxString& markup);

    // Returns entity parser object, used to substitute HTML &entities;
    wx28HtmlEntitiesParser *GetEntitiesParser() const { return m_entitiesParser; }

protected:
    // DOM structure
    void CreateDOMTree();
    void DestroyDOMTree();
    void CreateDOMSubTree(wx28HtmlTag *cur,
                          int begin_pos, int end_pos,
                          wx28HtmlTagsCache *cache);

    // Adds text to the output.
    // This is called from Parse() and must be overridden in derived classes.
    // txt is not guaranteed to be only one word. It is largest continuous part of text
    // (= not broken by tags)
    // NOTE : using char* because of speed improvements
    virtual void AddText(const wxChar* txt) = 0;

    // Adds tag and proceeds it. Parse() may (and usually is) called from this method.
    // This is called from Parse() and may be overridden.
    // Default behaviour is that it looks for proper handler in m_Handlers. The tag is
    // ignored if no hander is found.
    // Derived class is *responsible* for filling in m_Handlers table.
    virtual void AddTag(const wx28HtmlTag& tag);

protected:
    // DOM tree:
    wx28HtmlTag *m_CurTag;
    wx28HtmlTag *m_Tags;
    wx28HtmlTextPieces *m_TextPieces;
    size_t m_CurTextPiece;

    wxString m_Source;

    wx28HtmlParserState *m_SavedStates;

    // handlers that handle particular tags. The table is accessed by
    // key = tag's name.
    // This attribute MUST be filled by derived class otherwise it would
    // be empty and no tags would be recognized
    // (see wx28HtmlWinParser for details about filling it)
    // m_HandlersHash is for random access based on knowledge of tag name (BR, P, etc.)
    //      it may (and often does) contain more references to one object
    // m_HandlersList is list of all handlers and it is guaranteed to contain
    //      only one reference to each handler instance.
    wxList m_HandlersList;
    wxHashTable m_HandlersHash;

    wx28HtmlParser(const wx28HtmlParser&) = delete;
	wx28HtmlParser& operator=(const wx28HtmlParser&) = delete;

    // class for opening files (file system)
    wxFileSystem *m_FS;
    // handlers stack used by PushTagHandler and PopTagHandler
    wxList *m_HandlersStack;

    // entity parse
    wx28HtmlEntitiesParser *m_entitiesParser;

    // flag indicating that the parser should stop
    bool m_stopParsing;
};



// This class (and derived classes) cooperates with wx28HtmlParser.
// Each recognized tag is passed to handler which is capable
// of handling it. Each tag is handled in 3 steps:
// 1. Handler will modifies state of parser
//    (using its public methods)
// 2. Parser parses source between starting and ending tag
// 3. Handler restores original state of the parser
class  wx28HtmlTagHandler : public wxObject
{
    wxDECLARE_ABSTRACT_CLASS(wx28HtmlTagHandler);

public:
    wx28HtmlTagHandler() : wxObject () { m_Parser = NULL; }

    // Sets the parser.
    // NOTE : each _instance_ of handler is guaranteed to be called
    // only by one parser. This means you don't have to care about
    // reentrancy.
    virtual void SetParser(wx28HtmlParser *parser)
        { m_Parser = parser; }

    // Returns list of supported tags. The list is in uppercase and
    // tags are delimited by ','.
    // Example : "I,B,FONT,P"
    //   is capable of handling italic, bold, font and paragraph tags
    virtual wxString GetSupportedTags() = 0;

    // This is hadling core method. It does all the Steps 1-3.
    // To process step 2, you can call ParseInner()
    // returned value : true if it called ParseInner(),
    //                  false etherwise
    virtual bool HandleTag(const wx28HtmlTag& tag) = 0;

protected:
    // parses input between beginning and ending tag.
    // m_Parser must be set.
    void ParseInner(const wx28HtmlTag& tag)
        { m_Parser->DoParsing(tag.GetBeginPos(), tag.GetEndPos1()); }

    // Parses given source as if it was tag's inner code (see
    // wx28HtmlParser::GetInnerSource).  Unlike ParseInner(), this method lets
    // you specify the source code to parse. This is useful when you need to
    // modify the inner text before parsing.
    void ParseInnerSource(const wxString& source);

    wx28HtmlParser *m_Parser;

    wx28HtmlTagHandler(const wx28HtmlTagHandler&) = delete;
	wx28HtmlTagHandler& operator=(const wx28HtmlTagHandler&) = delete;
};


// This class is used to parse HTML entities in strings. It can handle
// both named entities and &#xxxx entries where xxxx is Unicode code.
class  wx28HtmlEntitiesParser : public wxObject
{
    wxDECLARE_DYNAMIC_CLASS(wx28HtmlEntitiesParser);

public:
    wx28HtmlEntitiesParser();
    virtual ~wx28HtmlEntitiesParser();

    // Sets encoding of output string.
    // Has no effect.
    void SetEncoding(wxFontEncoding encoding);

    // Parses entities in input and replaces them with respective characters
    // (with respect to output encoding)
    wxString Parse(const wxString& input);

    // Returns character for given entity or 0 if the enity is unknown
    wxChar GetEntityChar(const wxString& entity);

    // Returns character that represents given Unicode code
    wxChar GetCharForCode(unsigned code) { return (wxChar)code; }

protected:
    wx28HtmlEntitiesParser(const wx28HtmlEntitiesParser&) = delete;
	wx28HtmlEntitiesParser& operator=(const wx28HtmlEntitiesParser&) = delete;
};


#endif // _WX_HTMLPARS_H_
