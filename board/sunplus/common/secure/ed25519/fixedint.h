/*
    Portable header to provide the 32 and 64 bits type.

    Not a compatible replacement for <stdint.h>, do not blindly use it as such.
*/

#if ((defined(__STDC__) && __STDC__ && __STDC_VERSION__ >= 199901L) || (defined(__WATCOMC__) && (defined(_STDINT_H_INCLUDED) || __WATCOMC__ >= 1250)) || (defined(__GNUC__) && (defined(_STDINT_H) || defined(_STDINT_H_) || defined(__UINT_FAST64_TYPE__)) )) && !defined(FIXEDINT_H_INCLUDED)
    #include <linux/types.h>
    #undef FIXEDINT_H_INCLUDED
    #if defined(__WATCOMC__) && __WATCOMC__ >= 1250 && !defined(UINT64_C)
        #include <limits.h>
        #define UINT64_C(x) (x + (UINT64_MAX - UINT64_MAX))
    #endif
#endif


#ifndef FIXEDINT_H_INCLUDED
    #define FIXEDINT_H_INCLUDED
    /* (u)int64_t */
    #if (defined(__STDC__) && defined(__STDC_VERSION__) && __STDC__ && __STDC_VERSION__ >= 199901L)
        #define UINT64_C(v) v ##ULL
        #define INT64_C(v) v ##LL
    #elif defined(__GNUC__)
        #define UINT64_C(v) v ##ULL
        #define INT64_C(v) v ##LL
    #elif defined(__MWERKS__) || defined(__SUNPRO_C) || defined(__SUNPRO_CC) || defined(__APPLE_CC__) || defined(_LONG_LONG) || defined(_CRAYC)
        #define UINT64_C(v) v ##ULL
        #define INT64_C(v) v ##LL
    #elif (defined(__WATCOMC__) && defined(__WATCOM_INT64__)) || (defined(_MSC_VER) && _INTEGRAL_MAX_BITS >= 64) || (defined(__BORLANDC__) && __BORLANDC__ > 0x460) || defined(__alpha) || defined(__DECC)
        #define UINT64_C(v) v ##UI64
        #define INT64_C(v) v ##I64
    #endif
#endif
