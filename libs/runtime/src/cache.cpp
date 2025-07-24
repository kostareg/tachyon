#include "tachyon/runtime/cache.hpp"

namespace tachyon::runtime {
void Cache::put(Values vs, Value v) {
    const size_t entry_size_bytes = get_entry_size(vs, v);

    // 1. if the cache entry size by itself is already larger than the maximum size, we will
    // never cache it.
    if (entry_size_bytes > max_size_bytes) return;

    // 2. if the current size + entry size > the max size, evict the least recently used value
    //  until we have enough space. we erase it from both the map and the linked list.
    while (current_size_bytes + entry_size_bytes > max_size_bytes) {
        current_size_bytes -= get_entry_size(list.back().key, list.back().value);
        map.erase(list.back().key);
        list.pop_back();
    }

    current_size_bytes += entry_size_bytes;

    // 3. insert into map and list.
    // TODO: look into move semantics here. we are copying since we need both the datas.
    list.emplace_front(vs, v);
    map.insert_or_assign(vs, list.begin());
}

std::optional<Value> Cache::get(const Values &vs) {
    if (auto it = map.find(vs); it != map.end()) {
        // handle cache hit. move to the front of the list to record recent usage, then return
        // the cached value.
        list.splice(list.begin(), list, it->second);
        return list.front().value;
    }

    // cache miss.
    return std::nullopt;
};

size_t Cache::get_entry_size(const Values &vs, const Value &v) {
    // each list entry comes with 16-24 bytes of overhead, and each map entry comes with 8-16
    // bytes. let's record 24 + 16 as a "worst case".
    size_t entry_size_bytes = 40;

    // add the size of each key
    for (const Value &value : vs) {
        // since we are currently copying the keys, we should account for double the size
        entry_size_bytes += 2 * sizeof(Value);

        if (std::holds_alternative<std::string>(value)) {
            entry_size_bytes += std::get<std::string>(value).capacity();
        }
    }

    // add the size of the value
    entry_size_bytes += sizeof(Value);
    if (std::holds_alternative<std::string>(v)) {
        entry_size_bytes += std::get<std::string>(v).capacity();
    }

    return entry_size_bytes;
}
} // namespace tachyon::runtime