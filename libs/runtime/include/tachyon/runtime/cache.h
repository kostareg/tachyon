#pragma once

#include "tachyon/runtime/value.h"

#include <functional>
#include <list>
#include <unordered_map>

namespace tachyon::runtime
{
// from boost
inline void hash_combine(std::size_t &seed, std::size_t h)
{
    seed ^= h + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

struct ValueHash
{
    std::size_t operator()(Value const &v) const noexcept
    {
        return std::visit([]<typename T>(const T &val)
                          { return std::hash<std::decay_t<T>>{}(val); }, v);
    }
};

struct ValuesHash
{
    std::size_t operator()(Values const &vs) const noexcept
    {
        std::size_t h = vs.size();
        ValueHash vh;
        for (auto &v : vs)
            hash_combine(h, vh(v));
        return h;
    }
};

// TODO: should sizeof(Value) be constexpr?

/**
 * @brief a least recently used function memoization cache.
 *
 * Uses a map to store function arguments to values. Caps at TACHYON_MAX_CACHE_SIZE_MB
 * megabytes. One cache is stored per function, so we know the number of arguments at compile
 * time.
 *
 * TODO: explain why linked list
 */
class Cache
{
    struct Node
    {
        Values key;
        Value value;

        Node(Values key, Value value) : key(std::move(key)), value(std::move(value)) {}
    };

    std::list<Node> list;
    std::unordered_map<Values, std::list<Node>::iterator, ValuesHash> map;
    size_t current_size_bytes = sizeof(list) + sizeof(map);

    static constexpr size_t max_size_bytes = 500'000'000;

  public:
    Cache() = default;

    std::optional<Value> get(Values vs)
    {
        if (auto it = map.find(vs); it != map.end())
        {
            // handle cache hit. move to the front of the list to record recent usage, then return
            // the cached value.
            list.splice(list.begin(), list, it->second);
            return list.front().value;
        }

        // cache miss.
        return std::nullopt;
    };

    /**
     * @brief put item in cache, evicts least recently used if necessary
     *
     * Puts an item in the cache. If the size of the cache (map + linked list) is over
     * Cache::max_size_bytes, evict the least recently used values until we are under the limit.
     */
    void put(Values vs, Value v)
    {
        const size_t entry_size_bytes = getEntrySize(vs, v);

        // 2. if the current size + entry size > the max size, evict the least recently used value
        //  until we have enough space. we erase it from both the map and the linked list.
        while (current_size_bytes + entry_size_bytes > max_size_bytes)
        {
            current_size_bytes -= getEntrySize(list.back().key, list.back().value);
            map.erase(list.back().key);
            list.pop_back();
        }

        current_size_bytes += entry_size_bytes;

        // 3. insert into map and list.
        // TODO: look into move semantics here
        list.emplace_front(vs, v);
        map.insert_or_assign(vs, list.begin());
    }

    /**
     * @brief get size of an entry
     *
     * We approximate the size of the entry with the sum of:
     *   * sizeof(Value) for each key
     *   * sizeof(Value) for the one value
     *   * TODO: ??
     *   * any heap space allocated by std::string
     */
    static size_t getEntrySize(Values &vs, Value &v)
    {
        // each list entry comes with 16-24 bytes of overhead, and each map entry comes with 8-16
        // bytes. let's record 24 + 16 as a "worst case".
        size_t entry_size_bytes = 40;

        // handle the keys
        for (Value &value : vs)
        {
            // since we are currently copying the keys, we should account for double the size
            entry_size_bytes += 2 * sizeof(Value);

            if (std::holds_alternative<std::string>(value))
            {
                entry_size_bytes += std::get<std::string>(value).capacity();
            }
        }

        // handle the value
        entry_size_bytes += sizeof(Value);
        if (std::holds_alternative<std::string>(v))
        {
            entry_size_bytes += std::get<std::string>(v).capacity();
        }

        return entry_size_bytes;
    }
};
} // namespace tachyon::runtime