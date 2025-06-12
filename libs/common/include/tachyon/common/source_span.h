#pragma once

#include <cstddef>

namespace tachyon
{
/**
 * @brief represents a span of source code
 */
struct SourceSpan final
{
    /// source byte offset
    size_t position;
    /// source byte length
    size_t length;

    SourceSpan(size_t position, size_t length) : position(position), length(length) {}
};
} // namespace tachyon