///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/platinfo.cpp
// Purpose:     implements wxPlatformInfo class
// Author:      Francesco Montorsi
// Modified by:
// Created:     07.07.2006 (based on wxToolkitInfo)
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/utils.h"
#include "wx/apptrait.h"

#include <cassert>

module WX.Cmn.PlatInfo;

namespace
{

// global object
// VERY IMPORTANT: do not use the default constructor since it would
//                 try to init the wxPlatformInfo instance using
//                 gs_platInfo itself!
wxPlatformInfo gs_platInfo(wxPORT_UNKNOWN);

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// Keep "Unknown" entries to avoid breaking the indexes

const char* const wxOperatingSystemIdNames[] =
{
    "Apple Mac OS",
    "Apple Mac OS X",

    "Unknown", // STL build: cannot use _() to translate strings here
    "Microsoft Windows NT",
    "Unknown",
    "Unknown",

    "Linux",
    "FreeBSD",
    "OpenBSD",
    "NetBSD",

    "SunOS",
    "AIX",
    "HPUX",

    "Other Unix",
    "Other Unix",

    "Unknown",
    "Unknown",

};

const char* const wxPortIdNames[] =
{
    "wxBase",
    "wxMSW",
    "wxMotif",
    "wxGTK",
    "wxDFB",
    "wxX11",
    "Unknown",
    "wxMac",
    "wxCocoa",
    "Unknown",
    "wxQT"
};

const char* const wxBitnessNames[] =
{
    "32 bit",
    "64 bit"
};

// ----------------------------------------------------------------------------
// local functions
// ----------------------------------------------------------------------------

// returns the logarithm in base 2 of 'value'; this maps the enum values to the
// corresponding indexes of the string arrays above
unsigned wxGetIndexFromEnumValue(int value)
{
    wxCHECK_MSG( value, (unsigned)-1, "invalid enum value" );

    int n = 0;
    while ( !(value & 1) )
    {
        value >>= 1;
        n++;
    }

    wxASSERT_MSG( value == 1, "more than one bit set in enum value" );

    return n;
}

} // namespace anonymous

// ----------------------------------------------------------------------------
// wxPlatformInfo
// ----------------------------------------------------------------------------

wxPlatformInfo::wxPlatformInfo()
{
    // just copy platform info for currently running platform
    *this = Get();
}

wxPlatformInfo::wxPlatformInfo(wxPortId pid, int tkMajor, int tkMinor,
                               wxOperatingSystemId id, int osMajor, int osMinor,
                               wxBitness bitness,
                               std::endian endian,
                               bool usingUniversal)
    : m_tkVersionMajor(tkMajor),
      m_tkVersionMinor(tkMinor),
      m_port(pid),
      m_usingUniversal(usingUniversal),
      m_os(id),
      m_osVersionMajor(osMajor),
      m_osVersionMinor(osMinor),
      m_endian{endian},
      m_bitness(bitness)
{
}

bool wxPlatformInfo::operator==(const wxPlatformInfo &t) const
{
    return m_tkVersionMajor == t.m_tkVersionMajor &&
           m_tkVersionMinor == t.m_tkVersionMinor &&
           m_tkVersionMicro == t.m_tkVersionMicro &&
           m_osVersionMajor == t.m_osVersionMajor &&
           m_osVersionMinor == t.m_osVersionMinor &&
           m_osVersionMicro == t.m_osVersionMicro &&
           m_os == t.m_os &&
           m_osDesc == t.m_osDesc &&
           m_ldi == t.m_ldi &&
           m_desktopEnv == t.m_desktopEnv &&
           m_port == t.m_port &&
           m_usingUniversal == t.m_usingUniversal &&
           m_bitness == t.m_bitness &&
           m_endian == t.m_endian;
}

void wxPlatformInfo::InitForCurrentPlatform()
{
    m_initializedForCurrentPlatform = true;

    // autodetect all informations
    const wxAppTraits * const traits = wxApp::GetTraitsIfExists();
    if ( !traits )
    {
        wxFAIL_MSG( "failed to initialize wxPlatformInfo" );

        m_port = wxPORT_UNKNOWN;
        m_usingUniversal = false;
        m_tkVersionMajor =
        m_tkVersionMinor =
        m_tkVersionMicro = 0;
    }
    else
    {
        m_port = traits->GetToolkitVersion(&m_tkVersionMajor, &m_tkVersionMinor,
                                           &m_tkVersionMicro);
        m_usingUniversal = traits->IsUsingUniversalWidgets();
        m_desktopEnv = traits->GetDesktopEnvironment();
    }

    m_os = wxGetOsVersion(&m_osVersionMajor, &m_osVersionMinor, &m_osVersionMicro);
    m_osDesc = wxGetOsDescription();
    m_endian = std::endian::native;
    m_bitness = wxIsPlatform64Bit() ? wxBITNESS_64 : wxBITNESS_32;
    m_cpuArch = wxGetCpuArchitectureName();

#ifdef __LINUX__
    m_ldi = wxGetLinuxDistributionInfo();
#endif
    // else: leave m_ldi empty
}

/* static */
const wxPlatformInfo& wxPlatformInfo::Get()
{
    static bool initialized = false;
    if ( !initialized )
    {
        gs_platInfo.InitForCurrentPlatform();
        initialized = true;
    }

    return gs_platInfo;
}

/* static */
wxString wxPlatformInfo::GetOperatingSystemDirectory()
{
    return wxGetOSDirectory();
}



// ----------------------------------------------------------------------------
// wxPlatformInfo - enum -> string conversions
// ----------------------------------------------------------------------------

wxString wxPlatformInfo::GetOperatingSystemFamilyName(wxOperatingSystemId os)
{
    const char* string = "Unknown";
    if ( os & wxOS_MAC )
        string = "Macintosh";
    else if ( os & wxOS_WINDOWS )
        string = "Windows";
    else if ( os & wxOS_UNIX )
        string = "Unix";

    return string;
}

wxString wxPlatformInfo::GetOperatingSystemIdName(wxOperatingSystemId os)
{
    const unsigned idx = wxGetIndexFromEnumValue(os);

    wxCHECK_MSG( idx < WXSIZEOF(wxOperatingSystemIdNames), {},
                 "invalid OS id" );

    return wxOperatingSystemIdNames[idx];
}

wxString wxPlatformInfo::GetPortIdName(wxPortId port, bool usingUniversal)
{
    std::string ret;

    const unsigned idx = wxGetIndexFromEnumValue(port);

    wxCHECK_MSG( idx < WXSIZEOF(wxPortIdNames), ret,
                 "invalid port id" );

    ret = wxPortIdNames[idx];

    if ( usingUniversal )
        ret += "/wxUniversal";

    return ret;
}

wxString wxPlatformInfo::GetPortIdShortName(wxPortId port, bool usingUniversal)
{
    wxString ret;

    const unsigned idx = wxGetIndexFromEnumValue(port);

    wxCHECK_MSG( idx < WXSIZEOF(wxPortIdNames), ret,
                 "invalid port id" );

    ret = wxPortIdNames[idx];
    ret = ret.Mid(2).Lower();       // remove 'wx' prefix

    if ( usingUniversal )
        ret += "univ";

    return ret;
}

wxString wxPlatformInfo::GetBitnessName(wxBitness bitness)
{
    static_assert(WXSIZEOF(wxBitnessNames) == wxBITNESS_MAX,
                  "BitnessNames Mismatch (size)");

    return wxBitnessNames[bitness];
}

wxString wxPlatformInfo::GetEndiannessName(std::endian end)
{
    if(end == std::endian::little)
        return "Little endian";
    else if(end == std::endian::big)
        return "Big endian";
    else // Native; which could be of mixed endian type.
        return "Native endian";
}

bool wxPlatformInfo::CheckOSVersion(int major, int minor, int micro) const
{
    // If this instance of wxPlatformInfo has been initialized by InitForCurrentPlatform()
    // this check gets forwarded to the wxCheckOsVersion which might do more than a simple
    // number check if supported by the platform
    if (m_initializedForCurrentPlatform)
        return wxCheckOsVersion(major, minor, micro);
    else
        return DoCheckVersion(GetOSMajorVersion(),
                            GetOSMinorVersion(),
                            GetOSMicroVersion(),
                            major,
                            minor,
                            micro);
}

// ----------------------------------------------------------------------------
// wxPlatformInfo - string -> enum conversions
// ----------------------------------------------------------------------------

wxOperatingSystemId wxPlatformInfo::GetOperatingSystemId(const wxString &str)
{
    for ( size_t i = 0; i < WXSIZEOF(wxOperatingSystemIdNames); i++ )
    {
        if ( wxString(wxOperatingSystemIdNames[i]).CmpNoCase(str) == 0 )
            return (wxOperatingSystemId)(1 << i);
    }

    return wxOS_UNKNOWN;
}

wxPortId wxPlatformInfo::GetPortId(const wxString &str)
{
    // recognize both short and long port names
    for ( size_t i = 0; i < WXSIZEOF(wxPortIdNames); i++ )
    {
        const wxPortId current = (wxPortId)(1 << i);

        if ( wxString(wxPortIdNames[i]).CmpNoCase(str) == 0 ||
             GetPortIdShortName(current, true).CmpNoCase(str) == 0 ||
             GetPortIdShortName(current, false).CmpNoCase(str) == 0 )
            return current;
    }

    return wxPORT_UNKNOWN;
}

std::endian wxPlatformInfo::GetEndianness(const wxString& end)
{
    const wxString endl = end.Lower();

    if ( endl.StartsWith("little") )
        return std::endian::little;
    else if( endl.StartsWith("big") )
        return std::endian::big;
    else
        return std::endian::native;
}
