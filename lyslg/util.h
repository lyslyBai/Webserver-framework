#ifndef __LYSLG_UTIL_H__
#define __LYSLG_UTIL_H__

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <vector>

namespace lyslg{
pid_t GetThreadId();
uint64_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size = 64,int skip = 1);
std::string BacktraceToString(int size = 64,int skip = 2, const std::string& prefix = "");


}




#endif