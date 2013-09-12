#pragma once

#if defined(WIN32) || defined(_WIN32)
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
#endif

#include <cstdint>

// Types
typedef std::uint8_t UInt8;
typedef std::uint16_t UInt16;
typedef std::uint32_t UInt32;
typedef std::uint64_t UInt64;

typedef std::int8_t Int8;
typedef std::int16_t Int16;
typedef std::int32_t Int32;
typedef std::int64_t Int64;

#if INTPTR_MAX == INT32_MAX
typedef Int32 Int;
typedef UInt32 UInt;
#elif INTPTR_MAX == INT64_MAX
typedef Int64 Int;
typedef UInt64 UInt;
#else
#error "size of pointer is neither 32 or 64bit!"
#endif // INTPTR_MAX

