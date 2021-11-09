#ifndef _CONFTYPE_H_
#define _CONFTYPE_H_

import <algorithm>;
import <charconv>;
import <iostream>;
import <optional>;
import <utility>;
import <string>;
import <vector>;

namespace wx::cfg
{

enum class ReadValue
{
    Boolean,
    String,
    Double,
    Integer
};

using ConfigData = std::pair<std::string, ReadValue>;

template<typename KeyType, typename WriteType>
class ConfigContainer
{
public:
    using KeyT = KeyType;
    using WriteT = WriteType;

    void Write(const KeyT& key, const WriteType& val)
    {
        m_data.push_back({key, val});
    }

    void Set(const KeyT& key, const WriteType& val)
    {
        auto it = std::find_if(m_data.begin(), m_data.end(), [key](const auto& datum){ return datum.first == key; });

        if(it != m_data.end())
        {
            (*it).second = val;
        }
    }

    auto Read(const KeyT& key) const -> typename std::optional<typename std::tuple_element<0, WriteT>::type>
    {
        auto it = std::find_if(m_data.begin(), m_data.end(), [key](const auto& datum){ return datum.first == key; });
    
        if(it != m_data.end())
        {
            return {it->second.first};
        }

        return std::nullopt;
    }

    void Clear() noexcept
    {
        m_data.clear();
    }

private:
    std::vector<std::pair<KeyT, WriteType>> m_data;
};

template<class Containment, typename PathT = void>
class ConfigT
{
public:
    using KeyT = typename Containment::KeyT;
    using WriteT = typename Containment::WriteT;

    ConfigT() = default;

    ConfigT(Containment&& containment)
        : m_cont(std::move(containment))
    {

    }

    void Write(const KeyT& key, const WriteT& val)
    {
        m_cont.Write(key, val);
    }

    auto Read(const KeyT& key) const
    {
        return m_cont.Read(key);
    }

    void DeleteGroup(const KeyT& key)
    {

    }

    void DeleteAll()
    {
        m_cont.Clear();
    }

    bool containsGroup(const KeyT& key) const noexcept
    {

    }

    bool containsEntry(const KeyT& key) const noexcept
    {
        return m_cont.Contains(key);
    }


private:
    Containment m_cont;
};

} // namespace wx::config

#endif // _CONFTYPE_H_
