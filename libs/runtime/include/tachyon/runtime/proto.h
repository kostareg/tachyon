#pragma once

#include "tachyon/common/source_span.h"
#include "tachyon/runtime/value.h"

#include <vector>

namespace tachyon::runtime
{
struct Proto
{
    // TODO: uint8_t* + malloc
    std::vector<uint8_t> bytecode;

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

    size_t arguments;

    // debug info
    // TODO: actually using this
    std::string name;
    SourceSpan span;

    // TODO: when capturing vars from outer scope, just store a lookup table,
    //  don't store names
    // TODO: consider having `bool returnsVoid;` that indicates whether the
    //  0th register can stay the same (after the fn is called). Currently, it
    //  will overwrite it with garbage.
};
} // namespace tachyon::runtime