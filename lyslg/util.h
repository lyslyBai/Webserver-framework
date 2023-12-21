#ifndef __LYSLG_UTIL_H__
#define __LYSLG_UTIL_H__

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdint>

namespace lyslg{
pid_t GetThreadId();
uint32_t GetFiberId();


}




#endif