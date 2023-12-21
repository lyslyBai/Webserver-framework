#include <iostream>
#include <map>
#include <functional>
#include <time.h>
#include <string.h>
#include "log.h"

namespace lyslg{

LogEventWrap::LogEventWrap(LogEvent::ptr e)
    :m_event(e){

}
LogEventWrap::~LogEventWrap(){
    m_event->getLogger()->log(m_event->getLevel(),m_event);
}
std::stringstream& LogEventWrap::getSS(){
    return m_event->getSS();
}

void LogEvent::format(const char* fmt,...){
    va_list al;
    va_start(al,fmt);
    format(fmt,al);
    va_end(al);
}
void LogEvent::format(const char* fmt,va_list al){
    char* buf = nullptr;
    int len = vasprintf(&buf,fmt,al);
    if(len != -1){
        m_ss<<std::string(buf,len);
        free(buf);
    }
}


class MessageFormatItem:public LogFormatter::FormatItem
{
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << event->getContent();
    }
};

class LevelFormatItem:public LogFormatter::FormatItem
{
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem:public LogFormatter::FormatItem
{
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << event->getElapse();
    }
};

class NameFormatItem:public LogFormatter::FormatItem
{
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << logger->getName();
    }
};

class ThreadFormatItem:public LogFormatter::FormatItem
{
public:
    ThreadFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << event->getThreadId();
    }
};

class ThreadNameFormatItem:public LogFormatter::FormatItem
{
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << event->getThreadNmame();
    }
};

class FiberFormatItem:public LogFormatter::FormatItem
{
public:
    FiberFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem:public LogFormatter::FormatItem
{
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        m_format = "%Y-%m-%d %H:%M:%S";  // 这样写最终固定为这种格式了
    }
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time,&tm);
        char buf[64];
        // strptime(buf,m_format.c_str(),&tm); //用于将字符串解析为时间结构体。
        strftime(buf, sizeof(buf), m_format.c_str(), &tm); //用于将时间结构体格式化为字符串。
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem:public LogFormatter::FormatItem
{
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << event->getFile();
    }
};

class LineFormatItem:public LogFormatter::FormatItem
{
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << event->getLine();
    }
};

class NewlineFormatItem:public LogFormatter::FormatItem
{
public:
    NewlineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << std::endl;
    }
};

class TabFormatItem:public LogFormatter::FormatItem
{
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << "\t";
    }
};

class StringFormatItem:public LogFormatter::FormatItem
{
public:
    StringFormatItem(const std::string& str)
        :m_string(str){
        }
    void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override
    {
        os << m_string;
    }
private:
    std::string m_string;
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,int32_t line,int32_t elapse,int32_t threadId,int32_t fiberId,int64_t time)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(threadId)
    ,m_fiberId(fiberId)
    ,m_time(time)
    ,m_logger(logger)
    ,m_level(level){
        // std:: cout << "LogEvent" <<std::endl;
}

Logger::Logger(const std::string& name)  //%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
    :m_name(name)
    ,m_level(LogLevel::DEBUG) {
        // std:: cout << "Logger" <<std::endl;
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender){
    // std:: cout << "addAppender" <<std::endl;
    if(!appender->getformatter()){
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
    for(auto it = m_appenders.begin();it!=m_appenders.end();it++)
    {
        if(*it == appender){
            m_appenders.erase(it);
            break;
        }
    }
}


void Logger::log(LogLevel::Level level,LogEvent::ptr event){
    if(level >= m_level){
        auto self = shared_from_this();
        for(auto& i:m_appenders){
            i->log(self,level,event);
        }
    }
}

//#name是将name变成字符串的意思
const char* LogLevel::ToString(LogLevel::Level level)
{
    switch(level){
#define XX(name) \
    case LogLevel::name: \
        return #name; break;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

void Logger::debug(LogEvent::ptr event){
    log(LogLevel::DEBUG,event);
}
void Logger::info(LogEvent::ptr event){
    log(LogLevel::INFO,event);
}
void Logger::warn(LogEvent::ptr event){
     log(LogLevel::WARN,event);
}
void Logger::error(LogEvent::ptr event){
     log(LogLevel::ERROR,event);
}
void Logger::fatal(LogEvent::ptr event){
     log(LogLevel::FATAL,event);
}

FileLogAppender::FileLogAppender(const std::string& filename)
        :m_filename(filename){
        reopen();
}

bool FileLogAppender::reopen() 
{
    if(m_filestream){
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;
}

void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) 
{
    if(level >= m_level){
        m_filestream << m_formatter->format(logger,level,event);
    }
}
void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) 
{
    if(level >= m_level){
        std::cout << m_formatter->format(logger,level,event);
    }
}

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern){
        init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)
{
    std::stringstream ss;
    for(auto& i:m_items)
    {
        i->format(ss,logger,level,event);
    }
    return ss.str();
}

// "%d{%Y:%m:%d %H:%M:%s}%T%t%T%F%T{%p}%T{%c}%f:%l%T%m%n"
// %  %--(--) %%
// 日志格式的解析
void LogFormatter::init()
{
    // str fmt type
    std::vector<std::tuple<std::string,std::string,int>> vec;
    std::string nstr;
    for(size_t i = 0;i<m_pattern.length();i++)
    {
        if(m_pattern[i] != '%'){
            nstr.append(1,m_pattern[i]);
            continue;
        }
        if((i+1) < m_pattern.size() ){
            if(m_pattern[i+1]== '%'){
                nstr.append(1,'%');
                continue;
            }
        }

        size_t n = i+1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while(n < m_pattern.size())   //%d{%Y:%m:%d %H:%M:%s}
        {
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')){
                str = m_pattern.substr(i+1,n-i-1);
                break;
            }
            if(fmt_status == 0){
                if(m_pattern[n] == '{'){
                    str = m_pattern.substr(i+1,n-i-1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }else if(fmt_status == 1)
            {
                if(m_pattern[n] == '}'){
                    fmt = m_pattern.substr(fmt_begin+1,n-fmt_begin-1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size()){
                if(str.empty()){
                    str = m_pattern.substr(i+1);
                }
            }
        }
        if(fmt_status == 0){
            if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr,std::string(),0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str,fmt,1));
            i = n-1;
        }else if(fmt_status == 1){
            std::cout << "pattern parse error: " <<m_pattern<<"-"<<m_pattern.substr(i)<<std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>",fmt,0));
        }
    }
    if(!nstr.empty()){
        vec.push_back(std::make_tuple(nstr,"",0));
    }

    static std::map<std::string,std::function<FormatItem::ptr(const std::string& str)>> s_format_item = {
#define XX(str,C) {#str,[](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}
    XX(m,MessageFormatItem),   //消息体
    XX(p,LevelFormatItem),    //日志级别
    XX(r,ElapseFormatItem),    // 毫秒数
    XX(c,NameFormatItem),    // 日志名称
    XX(t,ThreadFormatItem),   // 线程Id
    XX(F,FiberFormatItem),    // 协程ID
    XX(n,NewlineFormatItem),   // 换行
    XX(d,DateTimeFormatItem),   // 日期
    XX(f,FilenameFormatItem),   // 文件名
    XX(l,LineFormatItem),    // 当前行
    XX(T,TabFormatItem),           // 制表符
    XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };

    // %m - 消息体
    // %p - levev
    // %r -- 启动后的时间
    // %c -- 日志名称
    // %t -- 线程id
    // %n -- 回车换行
    // %d -- 时间
    // %f -- 文件名
    // %l -- 行号
    // %T -- 制表符

    for(auto& i:vec){
        if(std::get<2>(i) == 0){
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }else{
            auto it = s_format_item.find(std::get<0>(i));
            if(it == s_format_item.end()){
                m_error = true;
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) +">>")));
            }else{
                m_items.push_back(it->second(std::get<1>(i)));
            }

        }
        // std::cout << "("<<  std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
   
}

LoggerManager::LoggerManager(){
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}
Logger::ptr LoggerManager::getLogger(const std::string name){
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root:it->second;
}




};