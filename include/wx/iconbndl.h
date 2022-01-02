///////////////////////////////////////////////////////////////////////////////
// Name:        wx/iconbndl.h
// Purpose:     wxIconBundle
// Author:      Mattia barbon
// Modified by:
// Created:     23.03.02
// Copyright:   (c) Mattia Barbon
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ICONBNDL_H_
#define _WX_ICONBNDL_H_

#include "wx/gdiobj.h"
#include "wx/icon.h"

import Utils.Geometry;

import <string>;
import <vector>;

class wxInputStream;

using wxIconArray = std::vector<wxIcon>;

// Load icons of multiple sizes from files or resources (MSW-only).
class wxIconBundle : public wxGDIObject
{
public:
    // Flags that determine what happens if GetIcon() doesn't find the icon of
    // exactly the requested size.
    enum class IconFallback
    {
        // Return invalid icon if exact size is not found.
        None,

        // Return the icon of the system icon size if exact size is not found.
        // May be combined with other non-NONE enum elements to determine what
        // happens if the system icon size is not found neither.
        System,

        // Return the icon of closest larger size or, if there is no icon of
        // larger size in the bundle, the closest icon of smaller size.
        NearestLarger
    };

    wxIconBundle() = default;

    // initializes the bundle with the icon(s) found in the file
#if wxUSE_STREAMS && wxUSE_IMAGE
#if wxUSE_FFILE || wxUSE_FILE
    wxIconBundle(const std::string& file, wxBitmapType type = wxBitmapType::Any);
#endif // wxUSE_FFILE || wxUSE_FILE
    wxIconBundle(wxInputStream& stream, wxBitmapType type = wxBitmapType::Any);
#endif // wxUSE_STREAMS && wxUSE_IMAGE

    // initializes the bundle with a single icon
    wxIconBundle(const wxIcon& icon);

#if defined(WX_WINDOWS) && wxUSE_ICO_CUR
    // initializes the bundle with the icons from a group icon stored as an MS Windows resource
    wxIconBundle(const std::string& resourceName, WXHINSTANCE module);
#endif

    // default copy ctor and assignment operator are OK

    // adds all the icons contained in the file to the collection,
    // if the collection already contains icons with the same
    // width and height, they are replaced
#if wxUSE_STREAMS && wxUSE_IMAGE
#if wxUSE_FFILE || wxUSE_FILE
    void AddIcon(const std::string& file, wxBitmapType type = wxBitmapType::Any);
#endif // wxUSE_FFILE || wxUSE_FILE
    void AddIcon(wxInputStream& stream, wxBitmapType type = wxBitmapType::Any);
#endif // wxUSE_STREAMS && wxUSE_IMAGE

#if defined(WX_WINDOWS) && wxUSE_ICO_CUR
    // loads all the icons from a group icon stored in an MS Windows resource
    void AddIcon(const std::string& resourceName, WXHINSTANCE module);
#endif

    // adds the icon to the collection, if the collection already
    // contains an icon with the same width and height, it is
    // replaced
    void AddIcon(const wxIcon& icon);

    // returns the icon with the given size; if no such icon exists,
    // behavior is specified by the flags.
    wxIcon GetIcon(const wxSize& size, IconFallback flags = IconFallback::System) const;

    // equivalent to GetIcon(wxSize(size, size))
    wxIcon GetIcon(wxCoord size = wxDefaultCoord,
                   IconFallback flags = IconFallback::System) const
        { return GetIcon(wxSize(size, size), flags); }

    // returns the icon exactly of the specified size or wxNullIcon if no icon
    // of exactly given size are available
    wxIcon GetIconOfExactSize(const wxSize& size) const;
    wxIcon GetIconOfExactSize(wxCoord size) const
        { return GetIconOfExactSize(wxSize(size, size)); }

    // enumerate all icons in the bundle: don't use these functions if ti can
    // be avoided, using GetIcon() directly is better

    // return the number of available icons
    size_t GetIconCount() const;

    // return the icon at index (must be < GetIconCount())
    wxIcon GetIconByIndex(size_t n) const;

    // check if we have any icons at all
    bool IsEmpty() const { return GetIconCount() == 0; }

protected:
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

private:
    // delete all icons
    void DeleteIcons();
};

inline const wxIconBundle wxNullIconBundle;

#endif // _WX_ICONBNDL_H_
