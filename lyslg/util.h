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


// 时间ms
uint16_t GetCurrentMS();
uint16_t GetCurrentUS();



class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);

};

}




#endif