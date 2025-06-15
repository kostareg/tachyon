#pragma once

#include <memory>
#include <print>
#include <string>
#include <variant>

namespace tachyon::runtime
{
// fwd-decl, see proto.h
struct Proto;

// TODO: currently 32 bytes, can we get less?
//  if we use char* copy = malloc(strlen(original) + 1), can store char* instead
//  and get significant decrease (should be 16 bytes). and if we can store
//  7 byte numbers, even if they need alignment, we will probably be at 8 bytes
//  total size.
// TODO: Proto is kept as a shared_ptr here. Consider other references (like having a storage pool
//  and keeping an int? or pointer?) and benchmark alternatives.
// TODO: can I/should I unify this value with the AST value, so that it's all
//  the same type? and put it in tachyon::common.

/// runtime value
using Value = std::variant<std::monostate, double, std::string, bool, std::shared_ptr<Proto>>;

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
            else
            {
                std::print("{}", val);
            }
        },
        reg);
}

} // namespace tachyon::runtime