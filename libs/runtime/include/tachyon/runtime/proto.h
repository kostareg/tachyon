#pragma once

#include "cache.h"
#include "tachyon/common/source_span.h"
#include "tachyon/runtime/value.h"

#include <utility>
#include <vector>

namespace tachyon::runtime
{
/**
 * @brief function prototype object
 */
struct Proto
{
    /// function bytecode instructions
    // TODO: uint16_t* + malloc
    std::vector<uint16_t> bytecode;

    // TODO: it could be an optimisation to store all constants somewhere
    //  else (eg in the VM) and call from there. There may be duplicate
    //  constants, and since they're currently stored in each Proto,
    //  duplicates within functions are not difficult but impossible to
    //  optimise since there is no way to reference them. You can also
    //  consider passing a value to the read constants instructions that
    //  indicate the depth of the constant, but that probably doesn't make
    //  sense.
    /**
     * @brief constant value lookup table
     */
    std::vector<Value> constants;

    /// number of arguments
    size_t arguments;

    /// whether the function is pure (memoizable)
    bool is_pure;

    /// memoization cache
    Cache cache;

    // TODO: actually using this:

    /// debug info name
    std::string name;

    /// debug info source span
    SourceSpan span;

    Proto() : bytecode(), constants(), arguments(), is_pure(), cache(), name(), span(0, 0) {}

    Proto(std::vector<uint16_t> bytecode, std::vector<Value> constants, size_t arguments,
          bool is_pure, std::string name, SourceSpan span)
        : bytecode(std::move(bytecode)), constants(std::move(constants)), arguments(arguments),
          is_pure(is_pure), cache(), name(std::move(name)), span(span)
    {
    }

    // TODO: when capturing vars from outer scope, just store a lookup table,
    //  don't store names
    // TODO: consider having `bool returnsVoid;` that indicates whether the
    //  0th register can stay the same (after the fn is called). Currently, it
    //  will overwrite it with garbage.
};
} // namespace tachyon::runtime