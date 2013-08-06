#pragma once

#include <cstdint>
#include <limits>

// Types
typedef std::uint8_t U8;
typedef std::uint16_t U16;
typedef std::uint32_t U32;
typedef std::uint64_t U64;

typedef std::int8_t S8;
typedef std::int16_t S16;
typedef std::int32_t S32;
typedef std::int64_t S64;

typedef float F32;
typedef double F64;

#if INTPTR_MAX == INT32_MAX
typedef S32 Int;
typedef U32 UInt;
#elif INTPTR_MAX == INT64_MAX
typedef S64 Int;
typedef U64 UInt;
#else
#error "size of pointer is neither 32 or 64bit!"
#endif // INTPTR_MAX

#ifndef USE_DOUBLE
typedef F32 FP;
#else
typedef F64 FP;
#endif

template<class T>
struct VF2T
{
    union
    {
        T x;
        T width;

        // aux
        T r;
        T i;
        T th;
    };
    union
    {
        T y;
        T height;

        // aux
        T g;
        T j;
        T ph;
    };
};

typedef VF2T<FP> VF2;

const S32 S32_MIN = std::numeric_limits<S32>::min();
const S32 S32_MAX = std::numeric_limits<S32>::max();
