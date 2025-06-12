#pragma once

#define TY_ALWAYS_ASSERT(...)                                                                      \
    do                                                                                             \
    {                                                                                              \
        if (__VA_ARGS__)                                                                           \
        {                                                                                          \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            ::std::cerr << "failed assertion in " << __FILE_NAME__ << " on line " << __LINE__      \
                        << ". in function " << __func__ << ".\n"                                   \
                        << #__VA_ARGS__;                                                           \
            ::std::abort();                                                                        \
        }                                                                                          \
    } while (false)

#define TY_NEVER_ASSERT(...)                                                                       \
    do                                                                                             \
    {                                                                                              \
        if (false)                                                                                 \
        {                                                                                          \
        }                                                                                          \
    } while (false)

#if defined(TY_DEBUG) && TY_DEBUG
#define TY_ASSERT(...) TY_ALWAYS_ASSERT(__VA_ARGS__)
#else
#define TY_ASSERT(...) TY_NEVER_ASSERT(__VA_ARGS__)
#endif