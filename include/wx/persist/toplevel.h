///////////////////////////////////////////////////////////////////////////////
// Name:        wx/persist/toplevel.h
// Purpose:     persistence support for wxTLW
// Author:      Vadim Zeitlin
// Created:     2009-01-19
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PERSIST_TOPLEVEL_H_
#define _WX_PERSIST_TOPLEVEL_H_

#include "wx/persist/window.h"

#include "wx/toplevel.h"

// ----------------------------------------------------------------------------
// string constants used by wxPersistentTLW
// ----------------------------------------------------------------------------

// we use just "Window" to keep configuration files and such short, there
// should be no confusion with wxWindow itself as we don't have persistent
// windows, just persistent controls which have their own specific kind strings
inline constexpr char wxPERSIST_TLW_KIND[] = "Window";

// ----------------------------------------------------------------------------
// wxPersistentTLW: supports saving/restoring window position and size as well
//                  as maximized/iconized/restore state
// ----------------------------------------------------------------------------

class wxPersistentTLW : public wxPersistentWindow<wxTopLevelWindow>,
                        private wxTopLevelWindow::GeometrySerializer
{
public:
    wxPersistentTLW(wxTopLevelWindow *tlw)
        : wxPersistentWindow<wxTopLevelWindow>(tlw)
    {
    }

    void Save() const override
    {
        const wxTopLevelWindow * const tlw = Get();

        tlw->SaveGeometry(*this);
    }

    bool Restore() override
    {
        wxTopLevelWindow * const tlw = Get();

        return tlw->RestoreToGeometry(*this);
    }

    std::string GetKind() const override { return wxPERSIST_TLW_KIND; }

private:
    bool SaveField(const std::string& name, int value) const override
    {
        return SaveValue(name, value);
    }

    bool RestoreField(const std::string& name, int* value) override
    {
        return RestoreValue(name, value);
    }
};

inline wxPersistentObject *wxCreatePersistentObject(wxTopLevelWindow *tlw)
{
    return new wxPersistentTLW(tlw);
}

#endif // _WX_PERSIST_TOPLEVEL_H_
