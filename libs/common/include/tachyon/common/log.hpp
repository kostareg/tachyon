#pragma once

#define TY_ALWAYS_TRACE(...)                                                                       \
    do {                                                                                           \
        std::cout << "[trace] " << __VA_ARGS__ << std::endl;                                       \
    } while (false)

#define TY_ALWAYS_TRACE_HEX(code, ...)                                                             \
    do {                                                                                           \
        std::cout << "[trace] " << __VA_ARGS__ << " 0x" << std::hex << code << std::dec            \
                  << std::endl;                                                                    \
    } while (false)

#define TY_NEVER_TRACE(...)                                                                        \
    do {                                                                                           \
    } while (false)

#define TY_NEVER_TRACE_HEX(code, ...)                                                              \
    do {                                                                                           \
    } while (false)

#if defined(TY_DEBUG) && TY_DEBUG
#include <iostream>
#define TY_TRACE(...) TY_ALWAYS_TRACE(__VA_ARGS__)
#define TY_TRACE_HEX(code, ...) TY_ALWAYS_TRACE_HEX(code, __VA_ARGS__)
#else
#define TY_TRACE(...) TY_NEVER_TRACE(__VA_ARGS__)
#define TY_TRACE_HEX(code, ...) TY_NEVER_TRACE_HEX(code, __VA_ARGS__)
#endif