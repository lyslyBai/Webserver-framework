#include <iostream>
#include "log.h"
#include "util.h"



// __FILE__ 和 __LINE__ 是 C++ 中的预定义宏，用于获取当前源文件名和行号

int main(int argc,char** argv)
{

    lyslg::Logger::ptr logger(new lyslg::Logger());
    logger->addAppender(lyslg::LogAppender::ptr(new lyslg::StdoutLogAppender()));

    // lyslg::LogEvent::ptr event(new lyslg::LogEvent(__FILE__,__LINE__,0,lyslg::GetThreadId(),lyslg::GetFiberId(),time(0)));
    // logger->log(lyslg::LogLevel::DEBUG, event);
    lyslg::FileLogAppender::ptr file_appender(new lyslg::FileLogAppender("./log.txt"));
    lyslg::LogFormatter::ptr fmt(new lyslg::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(lyslg::LogLevel::ERROR);
    logger->addAppender(file_appender);

    std::cout << "hello lyslg!" <<std::endl;

    LYSLG_LOG_INFO(logger) << "test macro";
    LYSLG_LOG_ERROR(logger) << "test macro error";

    LYSLG_LOG_FMT_ERROR(logger,"test macro fmt error %s","aa");
    
    auto l = lyslg::LoggerMgr::GetInstnce()->getLogger("xx");
    LYSLG_LOG_INFO(l) << "xxx";
    return 0;
}