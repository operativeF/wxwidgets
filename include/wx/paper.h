/////////////////////////////////////////////////////////////////////////////
// Name:        wx/paper.h
// Purpose:     Paper database types and classes
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PAPERH__
#define _WX_PAPERH__

#include "wx/event.h"
#include "wx/cmndata.h"
#include "wx/intl.h"
#include "wx/hashmap.h"

/*
 * Paper type: see printercfg.h for wxPaperSize enum.
 * A wxPrintPaperType can have an id and a name, or just a name and wxPaperSize::None,
 * so you can add further paper types without needing new ids.
 */

#ifdef __WXMSW__
#define WXADDPAPER(paperId, platformId, name, w, h) AddPaperType(paperId, platformId, name, w, h)
#else
#define WXADDPAPER(paperId, platformId, name, w, h) AddPaperType(paperId, 0, name, w, h)
#endif

class wxPrintPaperType
{
public:
    wxPrintPaperType() = default;

    // platformId is a platform-specific id, such as in Windows, DMPAPER_...
    wxPrintPaperType(wxPaperSize paperId, int platformId, std::string_view name, int w, int h);

    std::string GetName() const { return wxGetTranslation(m_paperName).ToStdString(); }
    wxPaperSize GetId() const { return m_paperId; }
    int GetPlatformId() const { return m_platformId; }

    // Get width and height in tenths of a millimetre
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    // Get size in tenths of a millimetre
    wxSize GetSize() const { return wxSize(m_width, m_height); }

    // Get size in a millimetres
    wxSize GetSizeMM() const { return wxSize(m_width/10, m_height/10); }

    // Get width and height in device units (1/72th of an inch)
    wxSize GetSizeDeviceUnits() const ;

public:
    std::string m_paperName;

    int         m_platformId{0};
    int         m_width{0};  // In tenths of a millimetre
    int         m_height{0}; // In tenths of a millimetre

    wxPaperSize m_paperId{wxPaperSize::None};
};

// FIXME: Move to string_hash file.
struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    size_t operator()(const char* str) const        { return hash_type{}(str); }
    size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

using wxStringToPrintPaperTypeHashMap = std::unordered_map<std::string, wxPrintPaperType*, string_hash, std::equal_to<>>;

class wxPrintPaperTypeList;

class wxPrintPaperDatabase
{
public:
    wxPrintPaperDatabase();
    ~wxPrintPaperDatabase();

    void CreateDatabase();
    void ClearDatabase();

    void AddPaperType(wxPaperSize paperId, std::string name, int w, int h);
    void AddPaperType(wxPaperSize paperId, int platformId, std::string name, int w, int h);

    // Find by name
    wxPrintPaperType *FindPaperType(std::string_view name) const;

    // Find by size id
    wxPrintPaperType *FindPaperType(wxPaperSize id) const;

    // Find by platform id
    wxPrintPaperType *FindPaperTypeByPlatformId(int id) const;

    // Find by size
    wxPrintPaperType *FindPaperType(const wxSize& size) const;

    // Convert name to size id
    wxPaperSize ConvertNameToId(std::string_view name) const;

    // Convert size id to name
    std::string ConvertIdToName(wxPaperSize paperId) const;

    // Get the paper size
    wxSize GetSize(wxPaperSize paperId) const;

    // Get the paper size
    wxPaperSize GetSize(const wxSize& size) const;

    //
    wxPrintPaperType* Item(size_t index) const;
    size_t GetCount() const;
private:
    wxStringToPrintPaperTypeHashMap* m_map;
    wxPrintPaperTypeList* m_list;
    //wxDECLARE_DYNAMIC_CLASS(wxPrintPaperDatabase);
};

inline wxPrintPaperDatabase* wxThePrintPaperDatabase{nullptr};


#endif
    // _WX_PAPERH__
