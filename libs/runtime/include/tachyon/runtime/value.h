#pragma once

#include "tachyon/runtime/matrix.h"

#include <memory>
#include <print>
#include <string>
#include <variant>
#include <vector>

namespace tachyon::runtime
{
// fwd-decl, see proto.h
struct Proto;

// TODO: currently 32 bytes, can we get less?
//  if we use char* copy = malloc(strlen(original) + 1), can store char* instead
//  and get significant decrease (should be 16 bytes). and if we can store
//  7 byte numbers, even if they need alignment, we will probably be at 8 bytes
//  total size. or maybe use string_view?
// TODO: Proto is kept as a shared_ptr here. Consider other references (like having a storage pool
//  and keeping an int? or pointer?) and benchmark alternatives.
// TODO: can I/should I unify this value with the AST value, so that it's all
//  the same type? and put it in tachyon::common.
// TODO: lists of non doubles

/// runtime value
using Value = std::variant<std::monostate, double, std::string, bool, std::shared_ptr<Proto>>;
using Values = std::vector<Value>;

/**
 * @brief small visitor for printing values
 */
inline void printValue(const Value &reg)
{
    std::visit(
        [](const auto &val)
        {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::monostate>)
            {
                std::print("()");
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<Proto>>)
            {
                std::print("proto");
            }
            else if constexpr (std::is_same_v<T, Matrix>)
            {
                for (const double &d : val)
                {
                    std::print("{}", d);
                }
            }
            else
            {
                std::print("{}", val);
            }
        },
        reg);
}

// TODO: research other hashing/hash combination functions

/**
 * @brief hash one value
 */
struct ValueHash
{
    std::size_t operator()(const Value &v) const noexcept
    {
        return std::visit([]<typename T>(const T &val)
                          { return std::hash<std::decay_t<T>>{}(val); }, v);
    }
};

/**
 * @brief hash combine values
 */
struct ValuesHash
{
    /**
     * @brief combine hashes
     *
     * Credit to:
     *   * https://www.boost.org/doc/libs/1_82_0/boost/container_hash/hash.hpp
     *   * https://www.boost.org/doc/libs/1_82_0/boost/container_hash/detail/hash_mix.hpp.
     *
     * @param seed seed
     * @param h value to combine
     */
    static void hash_combine(std::size_t &seed, std::size_t h)
    {
        seed ^= h + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    }

    std::size_t operator()(const Values &vs) const noexcept
    {
        std::size_t h = vs.size();
        ValueHash vh;
        for (auto &v : vs)
            hash_combine(h, vh(v));
        return h;
    }
};

} // namespace tachyon::runtime