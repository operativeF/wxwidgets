///////////////////////////////////////////////////////////////////////////////
// Name:        wx/iconloc.h
// Purpose:     declaration of wxIconLocation class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.06.2003
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ICONLOC_H_
#define _WX_ICONLOC_H_

#include <string>

// ----------------------------------------------------------------------------
// wxIconLocation: describes the location of an icon
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxIconLocationBase
{
public:
    // ctor takes the name of the file where the icon is
    explicit wxIconLocationBase(const std::string& filename = {})
        : m_filename(filename) { }


    // returns true if this object is valid/initialized
    bool IsOk() const { return !m_filename.empty(); }

    // set/get the icon file name
    void SetFileName(const std::string& filename) { m_filename = filename; }
    const std::string& GetFileName() const { return m_filename; }

private:
    std::string m_filename;
};

// under Windows the same file may contain several icons so we also store the
// index of the icon
#if defined(WX_WINDOWS)

class WXDLLIMPEXP_BASE wxIconLocation : public wxIconLocationBase
{
public:
    // ctor takes the name of the file where the icon is and the icons index in
    // the file
    explicit wxIconLocation(const std::string& file = {}, int num = 0);

    // set/get the icon index
    void SetIndex(int num) { m_index = num; }
    int GetIndex() const { return m_index; }

private:
    int m_index;
};

inline
wxIconLocation::wxIconLocation(const std::string& file, int num)
              : wxIconLocationBase(file)
{
    SetIndex(num);
}

#else // !WX_WINDOWS

// must be a class because we forward declare it as class
class WXDLLIMPEXP_BASE wxIconLocation : public wxIconLocationBase
{
public:
    explicit wxIconLocation(const std::string& filename = {})
        : wxIconLocationBase(filename) { }
};

#endif // platform

#endif // _WX_ICONLOC_H_

