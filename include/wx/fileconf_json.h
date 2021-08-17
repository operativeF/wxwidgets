#ifndef   _FILECONF_JSON_H
#define   _FILECONF_JSON_H

#include <filesystem>
#include <fstream>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <limits>

#include <boost/json/src.hpp>

namespace wx::cfg
{

namespace json = boost::json;
namespace fs = std::filesystem;

constexpr std::size_t DynamicExtentT = std::numeric_limits<std::size_t>::max();

template<typename Key, typename Value>
struct JsonEntry
{
    using key   = Key;
    using value = Value;
};

//template<std::size_t N = 4096, typename T = std::array<char, N>>
json::value ParseFile(const fs::path& file)
{
    std::ifstream f( file );

    json::stream_parser p;
    json::error_code ec;

    do
    {
        char buf[4095];
        f.read(buf, sizeof(buf));
        p.write( buf, f.gcount(), ec );
    } while( !f.eof() );

    if( ec )
        return {};

    p.finish( ec );

    if( ec )
        return {};

    return p.release();
}

// Add a parser value for a std::filesystem::path

class FileConfigJson
{
public:
    FileConfigJson(const fs::path& jsonFile)
    {
        m_parsedFile = ParseFile(jsonFile);
    }

    bool ContainsKey(const std::string& key) noexcept
    {
        return GetMainObject().contains(key);
    }

    bool ContainsSubkey(const std::string& key, const std::string& subkey)
    {
        auto primaryKey = GetMainObject().if_contains(key);

        if(primaryKey)
        {
            if(primaryKey->is_array())
            {
                auto& pkeyArray = primaryKey->get_array();

                for(auto& entry : pkeyArray)
                {
                    auto& ent = entry.as_object();

                    if(ent.contains(subkey))
                        return true;
                }
            }
        }

        return false;
    }

    template<typename Key, typename... SubKeys>
    auto AddEntry(const Key& key, const SubKeys&... subkeys, const json::value& entry)
    {
        return GetMainObject().insert_or_assign(key, entry);
    }

    void ReplaceEntry()
    {
        
    }

    void ClearEntry()
    {

    }

    void DeleteEntry()
    {

    }

    void TransmuteEntry()
    {

    }

    void Reset()
    {

    }

private:
    json::object& GetMainObject()
    {
        return m_parsedFile.as_object();
    }

    json::value m_parsedFile;

    fs::path m_file;
};

} // namespace wx::file

#endif // _FILECONF_JSON_H
