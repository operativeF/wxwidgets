///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/wrapshl.h
// Purpose:     wrapper class for stuff from shell32.dll
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2004-10-19
// Copyright:   (c) 2004 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_WRAPSHL_H_
#define _WX_MSW_WRAPSHL_H_

#include "wx/log.h"

#include <boost/nowide/convert.hpp>

// ----------------------------------------------------------------------------
// wxItemIdList implements RAII on top of ITEMIDLIST
// ----------------------------------------------------------------------------

class wxItemIdList
{
public:
    // ctor takes ownership of the item and will free it
    wxItemIdList(LPITEMIDLIST pidl)
    {
        m_pidl = pidl;
    }

    wxItemIdList& operator=(wxItemIdList&&) = delete;

    static void Free(LPITEMIDLIST pidl)
    {
        if ( pidl )
        {
            LPMALLOC pMalloc;
            std::ignore = SHGetMalloc(&pMalloc);
            if ( pMalloc )
            {
                pMalloc->Free(pidl);
                pMalloc->Release();
            }
            else
            {
                wxLogLastError("SHGetMalloc");
            }
        }
    }

    ~wxItemIdList()
    {
        Free(m_pidl);
    }

    // implicit conversion to LPITEMIDLIST
    operator LPITEMIDLIST() const { return m_pidl; }

    // get the corresponding path, returns empty string on error
    std::string GetPath() const
    {
        std::wstring path;

        path.resize(MAX_PATH);

        if ( !::SHGetPathFromIDListW(m_pidl, &path[0]) )
        {
            wxLogLastError("SHGetPathFromIDList");
        }

        return boost::nowide::narrow(path);
    }

private:
    LPITEMIDLIST m_pidl;
};

// enable autocompleting filenames in the text control with given WXHWND
//
// this only works on systems with shlwapi.dll 5.0 or later
//
// implemented in src/msw/utilsgui.cpp
extern bool wxEnableFileNameAutoComplete(WXHWND hwnd);

#endif // _WX_MSW_WRAPSHL_H_

