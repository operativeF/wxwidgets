/////////////////////////////////////////////////////////////////////////////
// Name:        wx/volume.h
// Purpose:     wxFSVolume - encapsulates system volume information
// Author:      George Policello
// Modified by:
// Created:     28 Jan 02
// Copyright:   (c) 2002 George Policello
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
// wxFSVolume represents a volume/drive in a file system
// ----------------------------------------------------------------------------

#ifndef _WX_FSVOLUME_H_
#define _WX_FSVOLUME_H_

#if wxUSE_FSVOLUME

import <string>;
import <vector>;

// the volume flags
enum wxFSVolumeFlags
{
    // is the volume mounted?
    wxFS_VOL_MOUNTED = 0x0001,

    // is the volume removable (floppy, CD, ...)?
    wxFS_VOL_REMOVABLE = 0x0002,

    // read only? (otherwise read write)
    wxFS_VOL_READONLY = 0x0004,

    // network resources
    wxFS_VOL_REMOTE = 0x0008
};

// the volume types
enum class wxFSVolumeKind
{
    Floppy,
    Disk,
    CDROM,
    DVDROM,
    Network,
    Other,
    Max
};

class wxFSVolumeBase
{
public:
    // return the array containing the names of the volumes
    //
    // only the volumes with the flags such that
    //  (flags & flagsSet) == flagsSet && !(flags & flagsUnset)
    // are returned (by default, all mounted ones)
    static std::vector<std::string> GetVolumes(unsigned int flagsSet = wxFS_VOL_MOUNTED,
                                            unsigned int flagsUnset = 0);

    // stop execution of GetVolumes() called previously (should be called from
    // another thread, of course)
    static void CancelSearch();

    // create the volume object with this name (should be one of those returned
    // by GetVolumes()).
    wxFSVolumeBase() = default;
    wxFSVolumeBase(const std::string& name);
    [[maybe_unused]] bool Create(const std::string& name);

    // is this a valid volume?
    bool IsOk() const;

    // kind of this volume?
    wxFSVolumeKind GetKind() const;

    // flags of this volume?
    int GetFlags() const;

    // can we write to this volume?
    bool IsWritable() const { return !(GetFlags() & wxFS_VOL_READONLY); }

    // get the name of the volume and the name which should be displayed to the
    // user
    std::string GetName() const { return m_volName; }
    std::string GetDisplayName() const { return m_dispName; }

    // TODO: operatios (Mount(), Unmount(), Eject(), ...)?

protected:
    // the internal volume name
    std::string m_volName;

    // the volume name as it is displayed to the user
    std::string m_dispName;

    // have we been initialized correctly?
    bool m_isOk{false};
};

#if wxUSE_GUI

#include "wx/icon.h"
#include "wx/iconbndl.h" // only for wxIconArray

enum wxFSIconType
{
    wxFS_VOL_ICO_SMALL = 0,
    wxFS_VOL_ICO_LARGE,
    wxFS_VOL_ICO_SEL_SMALL,
    wxFS_VOL_ICO_SEL_LARGE,
    wxFS_VOL_ICO_MAX
};

// wxFSVolume adds GetIcon() to wxFSVolumeBase
class wxFSVolume : public wxFSVolumeBase
{
public:
    wxFSVolume()  { InitIcons(); }
    wxFSVolume(const std::string& name) : wxFSVolumeBase(name) { InitIcons(); }

    wxIcon GetIcon(wxFSIconType type);

private:
    void InitIcons();

    // the different icons for this volume (created on demand)
    wxIconArray m_icons;
};

#else // !wxUSE_GUI

// wxFSVolume is the same thing as wxFSVolume in wxBase
using wxFSVolume = wxFSVolumeBase;

#endif // wxUSE_GUI/!wxUSE_GUI

#endif // wxUSE_FSVOLUME

#endif // _WX_FSVOLUME_H_
