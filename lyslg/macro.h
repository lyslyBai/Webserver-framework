#ifndef __LYSLG_MACRO_H__
#define __LYSLG_MACRO_H__

#include <assert.h>
#include <execinfo.h>
#include "util.h"

//  #if define __GNUC__ || define __11vm__
#if defined(__GNUC__) || defined(__11vm__)
#  define LYSLG_LICKLY(x)           __builtin_expect(!!(x),1)
#  define LYSLG_UNLICKLY(x)         __builtin_expect(!!(x),0)
#else
#  define LYSLG_LICKLY(x)               (x)
#  define LYSLG_UNLICKLY(x)             (x)
#endif

#define LYSLG_ASSERT(x) \
    if(LYSLG_UNLICKLY(!(x))) { \
        LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ASSERTION:" #x \
            << "\nbacktrace:\n" \
            << lyslg::BacktraceToString(100,2,"    "); \
        assert(x); \
    }

#define LYSLG_ASSERT2(x,w) \
    if(LYSLG_UNLICKLY(!(x))) { \
        LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ASSERTION:" #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << lyslg::BacktraceToString(100,2,"    "); \
        assert(x); \
    }



#endif