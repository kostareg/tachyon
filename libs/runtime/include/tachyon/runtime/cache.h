#pragma once

#include "tachyon/runtime/value.h"

#include <functional>
#include <list>
#include <unordered_map>

namespace tachyon::runtime
{
// TODO: should sizeof(Value) be constexpr?

/**
 * @brief a least recently used function memoization cache.
 *
 * Uses a linked list to store a list of arguments and function return values, and a map from
 * argument values to positions inside the list for fast lookup. When a value is put into the
 * cache, the value itself is placed at the front of the linked list, and a map entry is inserted
 * that records the list.front() iterator. It also evicts any old items from the back of the list,
 * if the estimated size exceeds Cache::max_size_bytes. When a value is retrieved from the cache,
 * and there is a cache hit, the value is retrieved from the linked list and the list node is moved
 * to the front.
 */
class Cache
{
    /**
     * @brief linked list node, stores function arguments and the return value
     */
    struct Node
    {
        /// function argument values
        Values key;

        /// function return value
        Value value;

        Node(Values key, Value value) : key(std::move(key)), value(std::move(value)) {}
    };

    /// node linked list, most recently used at front
    std::list<Node> list;

    /// map from argument values to positions in the linked list
    std::unordered_map<Values, std::list<Node>::iterator, ValuesHash> map;

    /// estimated current size of the cache
    size_t current_size_bytes = sizeof(list) + sizeof(map);

    /// approximate maximum size of the cache
    static constexpr size_t max_size_bytes = 500'000'000;

  public:
    Cache() = default;

    /**
     * @brief put item in cache, evict least recently used entries if necessary
     * @param vs argument list
     * @param v return value
     *
     * Puts an item in the cache. If the size of the cache (map + linked list) is over
     * Cache::max_size_bytes, evict the least recently used values until we are under the limit. If
     * the individual entry size is larger than the maximum size, do nothing.
     */
    void put(Values vs, Value v);

    /**
     * @brief try to get a cached value based on arguments list
     * @param vs argument list
     * @return cached return value if exists
     */
    std::optional<Value> get(const Values &vs);

  private:
    /**
     * @brief get size of an entry
     *
     * We approximate the size of the entry with the sum of:
     *   * sizeof(Value) for each key
     *   * sizeof(Value) for the one value
     *   * some approximate node overhead
     *   * any heap space allocated by std::string
     */
    static size_t getEntrySize(const Values &vs, const Value &v);
};
} // namespace tachyon::runtime