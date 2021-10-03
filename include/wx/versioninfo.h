///////////////////////////////////////////////////////////////////////////////
// Name:        wx/versioninfo.h
// Purpose:     declaration of wxVersionInfo class
// Author:      Troels K
// Created:     2010-11-22
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_VERSIONINFO_H_
#define _WX_VERSIONINFO_H_

#include <optional>

#include <fmt/core.h>

// ----------------------------------------------------------------------------
// wxVersionInfo: represents version information
// ----------------------------------------------------------------------------

struct VersionNumbering
{
    int major{};
    int minor{};
    std::optional<int> micro{};
};

class wxVersionInfo
{
public:
    constexpr wxVersionInfo(const std::string& name = {},
                  VersionNumbering versionNum = {},
                  const std::string& description = {},
                  const std::string& copyright = {})
        : m_name{name}
        , m_description{description}
        , m_copyright{copyright}
        , m_versionNum{versionNum}
    {
    }

    const std::string& GetName() const { return m_name; }

    const VersionNumbering& GetVersionNum() const
    {
        return m_versionNum;
    }

    std::string ToString() const
    {
        return !m_description.empty() ? GetDescription() : GetVersionString();
    }

    std::string GetVersionString() const
    {
        std::string optMicroVersion;

        if(m_versionNum.micro.has_value())
        {
            optMicroVersion = fmt::format(".{}", m_versionNum.micro.value());
        }

        return fmt::format("{} {}.{}{}", m_name,
                                         m_versionNum.major,
                                         m_versionNum.minor,
                                         optMicroVersion);
    }

    const std::string& GetDescription() const { return m_description; }
    const std::string& GetCopyright() const { return m_copyright; }

private:
    std::string m_name;
    std::string m_description;
    std::string m_copyright;

    VersionNumbering m_versionNum;
};

#endif // _WX_VERSIONINFO_H_
