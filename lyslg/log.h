#ifndef LYSLG_LOG_H
#define LYSLG_LOG_H

#include <string>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include <set>
#include "singleton.h"
#include "util.h"
#include "mutex.h"
#include "thread.h"

#define LYSLG_LOG_LEVEL(logger,level) \
    if(logger->getLevel() <= level) \
        lyslg::LogEventWrap(lyslg::LogEvent::ptr(new lyslg::LogEvent(logger,level \
        ,__FILE__,__LINE__,0,lyslg::GetThreadId(), \
        lyslg::GetFiberId(),time(0),lyslg::Thread::GetName()))).getSS()

#define LYSLG_LOG_DEBUG(logger) LYSLG_LOG_LEVEL(logger,lyslg::LogLevel::DEBUG)
#define LYSLG_LOG_INFO(logger) LYSLG_LOG_LEVEL(logger,lyslg::LogLevel::INFO)
#define LYSLG_LOG_WARN(logger) LYSLG_LOG_LEVEL(logger,lyslg::LogLevel::WARN)
#define LYSLG_LOG_ERROR(logger) LYSLG_LOG_LEVEL(logger,lyslg::LogLevel::ERROR)
#define LYSLG_LOG_FATAL(logger) LYSLG_LOG_LEVEL(logger,lyslg::LogLevel::FATAL)

#define LYSLG_LOG_FMT_LEVEL(logger,level,fmt,...) \
    if(logger->getLevel() <= level) \
        lyslg::LogEventWrap(lyslg::LogEvent::ptr(new lyslg::LogEvent(logger,level \
        ,__FILE__,__LINE__,0,lyslg::GetThreadId(),lyslg::GetFiberId(), \
        time(0),lyslg::Thread::GetName()))).getEvent()->format(fmt,__VA_ARGS__)                    // __VA_ARGS__ 等价于 ... 内容，为预定义的宏

#define LYSLG_LOG_FMT_DEBUG(logger,fmt,...) LYSLG_LOG_FMT_LEVEL(logger,lyslg::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define LYSLG_LOG_FMT_INFO(logger,fmt,...) LYSLG_LOG_FMT_LEVEL(logger,lyslg::LogLevel::INFO,fmt,__VA_ARGS__)
#define LYSLG_LOG_FMT_WARN(logger,fmt,...) LYSLG_LOG_FMT_LEVEL(logger,lyslg::LogLevel::WARN,fmt,__VA_ARGS__)
#define LYSLG_LOG_FMT_ERROR(logger,fmt,...) LYSLG_LOG_FMT_LEVEL(logger,lyslg::LogLevel::ERROR,fmt,__VA_ARGS__)
#define LYSLG_LOG_FMT_FATAL(logger,fmt,...) LYSLG_LOG_FMT_LEVEL(logger,lyslg::LogLevel::FATAL,fmt,__VA_ARGS__)

#define LYSLG_LOG_ROOT() lyslg::LoggerMgr::GetInstance()->getRoot()
#define LYSLG_LOG_NAME(name) lyslg::LoggerMgr::GetInstance()->getLogger(name)

namespace lyslg{


class Logger;
class LoggerManager;

// 日志级别
class LogLevel
{
public:
    enum Level{
        UNKNOW = 0,
        DEBUG = 1,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& str);
};

class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, \
        const char* file,int32_t line,int32_t elapse,int32_t threadId, \
        int32_t fiberId,int64_t time, const std::string& thread_name);

    const char* getFile() const {return m_file;}
    int32_t getLine() const {return m_line;}
    int32_t getElapse() const {return m_elapse;}
    int32_t getThreadId() const {return m_threadId;}
    std::string getThreadName() const {return m_threadName;}
    int32_t getFiberId() const {return m_fiberId;}
    int64_t getTime() const {return m_time;}
    std::string getContent() const {return m_ss.str();}
    std::stringstream& getSS() {return m_ss;}  // 这里返回引用，且不能使用const
    std::shared_ptr<Logger> getLogger() const {return m_logger;}
    LogLevel::Level getLevel() const {return m_level;}

    void format(const char* fmt,...);
    void format(const char* fmt,va_list al);
private:
    const char* m_file = nullptr; // 文件名
    int32_t m_line = 0;   // 行号
    int32_t m_elapse = 0;  // 程序启动到现在的毫秒数
    int32_t m_threadId = 0; // 线程ID
    int32_t m_fiberId = 0; // 协程ID
    int64_t m_time = 0; // 时间戳
    std::stringstream m_ss; // 
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    std::string m_threadName ; // 线程名称
};

class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const { return m_event;}
    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;
};

// 规定日志的输出格式
class LogFormatter
{
friend class Logger;
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);
    LogFormatter(const std::string& pattern);

public:
    // 格式解析模块
    class FormatItem
    {
    public: 
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {};
        virtual void format(std::ostream& ss,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) = 0;
    };
    void init();
    bool isError() const {return m_error;}

    const std::string getPattern() const {return m_pattern;}
private:
    std::vector<FormatItem::ptr> m_items;
    std::string m_pattern;
    /// 是否有错误
    bool m_error = false;
};

// 指定日志的输出地点
class LogAppender
{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr; 
    typedef Mutex MutexType;
    virtual void log(std::shared_ptr<Logger>,LogLevel::Level level,LogEvent::ptr event) = 0; // 子类重载

    LogFormatter::ptr getFormatter();
    void setFormatter(LogFormatter::ptr formatter);

    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level level) {m_level = level;}

    virtual ~LogAppender() {} ;

    virtual std::string toYamlString() = 0;

protected:   // 在子类中可能会用到，故使用protected
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
    MutexType m_mutex;
    bool m_hasFormatter = false;
};

// 日志器 Mutex日志写入的速度最快，我还是使用这个吧
class Logger :public std::enable_shared_from_this<Logger> // 可以使用shared_from_this(); 获取自身的shared_ptr
{ 
friend class  LoggerManager;
public: 
    typedef std::shared_ptr<Logger> ptr;
    typedef Mutex MutexType;
    Logger(const std::string& name = "root");

    void log(LogLevel::Level level,LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppender();

    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level val) {m_level = val;}

    std::string getName() const {return m_name;}
    void setName(std::string name) {m_name = name;}

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();

    std::string toYamlString();

private:
    std::string m_name; // 日志名称
    LogLevel::Level m_level;  //日志级别
    std::list<LogAppender::ptr> m_appenders;  // 日志输出地点，为一个列表
    LogFormatter::ptr m_formatter;
    Logger::ptr m_root;
    MutexType m_mutex;
};



// 输出到控制台的Appender
class StdoutLogAppender: public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    std::string toYamlString() override;
    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;
};
// 输出到文件的Appender
class FileLogAppender: public LogAppender
{
public: 
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    bool reopen();
    std::string toYamlString() override;
    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;
private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime;
};

class LoggerManager
{
public:
    typedef Mutex MutexType;
    LoggerManager();
    Logger::ptr getLogger(const std::string name);

    void init();
    std::string toYamlString();

    Logger::ptr getRoot() const {return m_root;}
private:
    std::map<std::string,Logger::ptr> m_loggers;
    Logger::ptr m_root;
    MutexType m_mutex;
};

typedef lyslg::Singleton<LoggerManager> LoggerMgr;

}

#endif
