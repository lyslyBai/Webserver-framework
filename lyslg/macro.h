#ifndef __LYSLG_MACRO_H__
#define __LYSLG_MACRO_H__

#include <assert.h>
#include <execinfo.h>
#include "util.h"

#define LYSLG_ASSERT(x) \
    if(!(x)) { \
        LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ASSERTION:" #x \
            << "\nbacktrace:\n" \
            << lyslg::BacktraceToString(100,2,"    "); \
        assert(x); \
    }

#define LYSLG_ASSERT2(x,w) \
    if(!(x)) { \
        LYSLG_LOG_ERROR(LYSLG_LOG_ROOT()) << "ASSERTION:" #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << lyslg::BacktraceToString(100,2,"    "); \
        assert(x); \
    }



#endif