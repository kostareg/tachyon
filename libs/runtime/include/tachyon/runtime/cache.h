#pragma once

#include "tachyon/runtime/value.h"

#include <functional>
#include <unordered_map>

namespace tachyon::runtime
{
/**
 * @brief maximum items in the cache (500 MB)
 *
 * As of the time of writing, the size of a Value is 40 bytes on most systems. We are looking to
 * cap the memory usage around 500 MB - however, this includes both cache keys and values. Assuming
 * an average entry size of approx. 270 B (avg. 3 keys + 1 value, where each Value is ~40 bytes),
 * we find that the maximum number of cache items is 500 MB / 270 B = ~1'851'851.
 */
inline constexpr std::size_t MaxCacheItems = 1'800'000;

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

struct FunctionArgumentsHash
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

/**
 * @brief a least recently used function memoization cache.
 *
 * Uses a map to store function arguments to values. Caps at TACHYON_MAX_CACHE_SIZE_MB
 * megabytes. One cache is stored per function, so we know the number of arguments at compile
 * time.
 */
class Cache
{
    std::unordered_map<Values, Value, FunctionArgumentsHash> map;

  public:
    Cache() = default;

    std::optional<Value> get(Values vs)
    {
        if (auto it = map.find(vs); it != map.end())
            return it->second;
        return std::nullopt;
    };

    void set(Values vs, Value v) { map.insert_or_assign(vs, v); }
};
} // namespace tachyon::runtime