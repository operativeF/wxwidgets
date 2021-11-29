#ifndef _CONFREG_H_
#define _CONFREG_H_

#include "wx/config_type.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

#include <winreg/WinReg.hpp>

namespace wx::cfg
{

// TODO: Add local machine / local user disambiguation.
class RegistryContainer
{
public:
    RegistryContainer(const std::string& vendor,
                      const std::string& appname)
        : m_basekey{HKEY_CURRENT_USER,
                    boost::nowide::widen("SOFTWARE\\" + vendor + '\\' + appname)}
    {
    }

    // Create a new subkey and assign to it an optional value (otherwise default)
    void Create([[maybe_unused]] const std::string& subkey, [[maybe_unused]] const std::string& val = {})
    {
        
    }

    // Write a value to the current subkey.
    void SetString(std::string_view value, std::string_view data)
    {
        m_basekey.SetStringValue(boost::nowide::widen(value), boost::nowide::widen(data));
    }

    void SetDword(const std::string& value, DWORD data)
    {
        m_basekey.SetDwordValue(boost::nowide::widen(value), data);
    }

    void SetMultiStringValue(std::string_view value, const std::vector<std::string>& data)
    {
        std::vector<std::wstring> wdata;
        wdata.reserve(data.size());

        std::ranges::transform(data, std::back_inserter(wdata),
            [](const auto& str){ return boost::nowide::widen(str); });

        m_basekey.SetMultiStringValue(boost::nowide::widen(value), wdata);
    }

    // Try to read a value from a subkey.
    std::optional<std::string> ReadString(std::string_view subkey) const
    {
        auto possibleStr = m_basekey.TryGetStringValue(boost::nowide::widen(subkey));

        if(possibleStr)
            return boost::nowide::narrow(possibleStr.value());

        return std::nullopt;
    }

    // Like the WinReg EnumSubkeys, except it returns a string instead.
    std::vector<std::string> EnumerateSubkeys() const
    {
        _ASSERTE(IsValid());

        // Get some useful enumeration info, like the total number of subkeys
        // and the maximum length of the subkey names
        DWORD subKeyCount = 0;
        DWORD maxSubKeyNameLen = 0;
        LONG retCode = ::RegQueryInfoKeyW(
            m_basekey.Get(),
            nullptr,    // no user-defined class
            nullptr,    // no user-defined class size
            nullptr,    // reserved
            &subKeyCount,
            &maxSubKeyNameLen,
            nullptr,    // no subkey class length
            nullptr,    // no value count
            nullptr,    // no value name max length
            nullptr,    // no max value length
            nullptr,    // no security descriptor
            nullptr     // no last write time
        );
        if (retCode != ERROR_SUCCESS)
        {
            throw winreg::RegException{
                retCode,
                "RegQueryInfoKeyW failed while preparing for subkey enumeration."
            };
        }

        // NOTE: According to the MSDN documentation, the size returned for subkey name max length
        // does *not* include the terminating NUL, so let's add +1 to take it into account
        // when I allocate the buffer for reading subkey names.
        maxSubKeyNameLen++;

        // Preallocate a buffer for the subkey names
        auto nameBuffer = std::make_unique<wchar_t[]>(maxSubKeyNameLen);

        // The result subkey names will be stored here
        std::vector<std::string> subkeyNames;

        // Reserve room in the vector to speed up the following insertion loop
        subkeyNames.reserve(subKeyCount);

        // Enumerate all the subkeys
        for (DWORD index = 0; index < subKeyCount; index++)
        {
            // Get the name of the current subkey
            DWORD subKeyNameLen = maxSubKeyNameLen;
            retCode = ::RegEnumKeyExW(
                m_basekey.Get(),
                index,
                nameBuffer.get(),
                &subKeyNameLen,
                nullptr, // reserved
                nullptr, // no class
                nullptr, // no class
                nullptr  // no last write time
            );
            if (retCode != ERROR_SUCCESS)
            {
                throw winreg::RegException{ retCode, "Cannot enumerate subkeys: RegEnumKeyExW failed." };
            }

            // On success, the ::RegEnumKeyEx API writes the length of the
            // subkey name in the subKeyNameLen output parameter
            // (not including the terminating NUL).
            // So I can build a wstring based on that length.
            subkeyNames.emplace_back(boost::nowide::narrow(nameBuffer.get(), subKeyNameLen));
        }

        return subkeyNames;
    }

    // Like the WinReg EnumValues, except it returns a string instead.
    std::vector<std::pair<std::string, DWORD>> EnumerateValues() const
    {
        _ASSERTE(IsValid());

        // Get useful enumeration info, like the total number of values
        // and the maximum length of the value names
        DWORD valueCount = 0;
        DWORD maxValueNameLen = 0;
        LONG retCode = ::RegQueryInfoKeyW(
            m_basekey.Get(),
            nullptr,    // no user-defined class
            nullptr,    // no user-defined class size
            nullptr,    // reserved
            nullptr,    // no subkey count
            nullptr,    // no subkey max length
            nullptr,    // no subkey class length
            &valueCount,
            &maxValueNameLen,
            nullptr,    // no max value length
            nullptr,    // no security descriptor
            nullptr     // no last write time
        );
        if (retCode != ERROR_SUCCESS)
        {
            throw winreg::RegException{
                retCode,
                "RegQueryInfoKeyW failed while preparing for value enumeration."
            };
        }

        // NOTE: According to the MSDN documentation, the size returned for value name max length
        // does *not* include the terminating NUL, so let's add +1 to take it into account
        // when I allocate the buffer for reading value names.
        maxValueNameLen++;

        // Preallocate a buffer for the value names
        auto nameBuffer = std::make_unique<wchar_t[]>(maxValueNameLen);

        // The value names and types will be stored here
        std::vector<std::pair<std::string, DWORD>> valueInfo;

        // Reserve room in the vector to speed up the following insertion loop
        valueInfo.reserve(valueCount);

        // Enumerate all the values
        for (DWORD index = 0; index < valueCount; index++)
        {
            // Get the name and the type of the current value
            DWORD valueNameLen = maxValueNameLen;
            DWORD valueType = 0;
            retCode = ::RegEnumValueW(
                m_basekey.Get(),
                index,
                nameBuffer.get(),
                &valueNameLen,
                nullptr,    // reserved
                &valueType,
                nullptr,    // no data
                nullptr     // no data size
            );

            if (retCode != ERROR_SUCCESS)
            {
                throw winreg::RegException{ retCode, "Cannot enumerate values: RegEnumValueW failed." };
            }

            // On success, the RegEnumValue API writes the length of the
            // value name in the valueNameLen output parameter
            // (not including the terminating NUL).
            // So we can build a string based on that.
            valueInfo.emplace_back(
                boost::nowide::narrow( nameBuffer.get(), valueNameLen ),
                valueType
            );
        }

        return valueInfo;
    }

    // Delete the whole subkey directory (uninstallation)
    void Clear()
    {
        m_basekey.DeleteTree(L"");
    }

private:
    winreg::RegKey m_basekey;
};

} // namespace wx::cfg


#endif // _CONFREG_H_
