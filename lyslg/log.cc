#include <iostream>
#include <map>
#include <functional>
#include <time.h>
#include <string.h>
#include "config.h"  // 这个头文件不能包含在log.h中，
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
        os << event->getLogger()->getName();
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
    if(!appender->getFormatter()){
        appender->m_formatter = m_formatter;
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
        if(!m_appenders.empty()){
            for(auto& i:m_appenders){
                i->log(self,level,event);
            }
        }else{
            m_root->log(level,event);
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

LogLevel::Level LogLevel::FromString(const std::string& str)
{
#define XX(level,name) \
    if(str == #name) { \
        return LogLevel::level; \
    }
    XX(DEBUG,debug);
    XX(INFO,info);
    XX(WARN,warn);
    XX(ERROR,error);
    XX(FATAL,fatal);

    XX(DEBUG,DEBUG);
    XX(INFO,INFO);
    XX(WARN,WARN);
    XX(ERROR,ERROR);
    XX(FATAL,FATAL);

    return LogLevel::UNKNOW;
#undef XX
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

void LogAppender::setFormatter(LogFormatter::ptr formatter){
    m_formatter = formatter;
    if(m_formatter){
        m_hasFormatter = true;
    } else {
        m_hasFormatter = false;
    }

}

bool FileLogAppender::reopen() 
{
    if(m_filestream){
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;
}

std::string FileLogAppender::toYamlString() {
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level){
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
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

std::string StdoutLogAppender::toYamlString() {
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level){
        node["level"] = LogLevel::ToString(m_level);
    }
    
    if(m_hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
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

void Logger::clearAppender(){
    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr val){
    m_formatter = val;
    for(auto& i:m_appenders) {
        if(!i->m_hasFormatter) {
            i->m_formatter = m_formatter;
        }
    }
}
void Logger::setFormatter(const std::string& val){
    lyslg::LogFormatter::ptr new_val(new lyslg::LogFormatter(val));
    if(new_val->isError() ) {
        std::cout << "Log setFormatter name=" <<m_name
                  << " value=" << val << " invalid formatter"
                  << std::endl;
        return;
    }
    // m_formatter = new_val;
    setFormatter(new_val);
}

LogFormatter::ptr Logger::getFormatter() {
    return m_formatter;
}

LoggerManager::LoggerManager(){
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

    m_loggers[m_root->getName()] = m_root;
    init();
}
Logger::ptr LoggerManager::getLogger(const std::string name){
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()){
        return it->second;
    }
    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;

}

std::string Logger::toYamlString() {
    YAML::Node node;
    node["name"] = m_name;
    node["level"] = LogLevel::ToString(m_level);
    if(m_formatter ) {
        node["formatter"] = m_formatter->getPattern();
    }

    for(auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }

    std::stringstream ss;
    ss << node;
    return ss.str();
}

struct LogAppenderDefine{
    int type = 0; // 1 File 2 Stdout
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;
    bool operator==(const LogAppenderDefine& oth) const {
        return oth.type == type
            && oth.formatter == formatter
            && oth.level == level
            && oth.file == file;
    }
};

struct LogDefine{
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;
    bool operator==(const LogDefine& oth) const {
        return oth.appenders == appenders
            && oth.formatter == formatter
            && oth.level == level
            && oth.name == name;
    }

    bool operator<(const LogDefine& oth) const {
        return name < oth.name;
    }
};

template<>
class LexicalCast<std::string, std::set<LogDefine>> {
public:
   std::set<LogDefine> operator()(const std::string& v){
        YAML::Node node = YAML::Load(v);
        std::set<LogDefine> vec;
        for(size_t i = 0;i < node.size(); i++) {
            auto n = node[i];
            if(!n["name"].IsDefined()) {
                std::cout << "log config error name is null, " << n     
                          << std::endl;
                continue;
            }

            LogDefine Id;
            Id.name = n["name"].as<std::string>();
            Id.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
            if(n["formatter"].IsDefined()){
                Id.formatter = n["formatter"].as<std::string>();
            }

            if(n["appenders"].IsDefined()) {
                for(size_t x=0;x < n["appenders"].size();++x) {
                    auto a = n["appenders"][x];
                    if(!a["type"].IsDefined()) {
                        std::cout << "log config error: appender type is null, " << a     
                            << std::endl;
                        continue;
                    }
                    std::string type = a["type"].as<std::string>();
                    LogAppenderDefine lad;
                    if(type == "FileLogAppender") {
                        lad.type = 1;
                        if(!a["file"].IsDefined()){
                            std::cout << "log config error: fileappender file is null, " << a     
                            << std::endl;
                            continue;
                        }
                        lad.file = a["file"].as<std::string>();
                    }else if(type == "StdoutLogAppender"){
                        lad.type = 2;
                    }else{
                        std::cout << "log config error: appender type is invalid, " << a     
                            << std::endl;
                        continue;
                    }
                    if(a["formatter"].IsDefined()){
                            lad.formatter = a["formatter"].as<std::string>();
                           //  std::cout << "lad.formatter"<< lad.formatter << std::endl;
                    }
                    lad.level = LogLevel::FromString(a["level"].IsDefined() ? a["level"].as<std::string>() : "");
                    Id.appenders.push_back(lad);
                }
            }
            vec.insert(Id);
        }
        return vec;
    }
};


template<>
class LexicalCast<std::set<LogDefine>, std::string> {
public:
    std::string operator()(const std::set<LogDefine>& v){
        YAML::Node node;
        for(auto& i : v){
            YAML::Node n;
            n["name"] = i.name;
            if(i.level != LogLevel::UNKNOW) {
                 n["level"] = LogLevel::ToString(i.level);
            }
           
            if(!i.formatter.empty()){
                n["m_formatter"] = i.formatter;
            }

            for(auto& a:i.appenders){
                YAML::Node na;
                if(a.type == 1) {
                    na["type"] = "FileLogAppender";
                    na["file"] = a.file;
                } else if(a.type == 2) {
                    na["type"] = "StdoutLogAppender";
                }
                if(i.level != LogLevel::UNKNOW) {
                     na["level"] = LogLevel::ToString(a.level);
                }
                if(!a.formatter.empty()){
                    na["m_formatter"] = a.formatter;
                }
                n["appenders"].push_back(na);
            }
            node.push_back(n);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


lyslg::ConfigVar<std::set<LogDefine> >::ptr g_log_define =
    lyslg::Config::Lookup("logs",std::set<LogDefine>(), "logs config");

struct LogIniter{
    LogIniter() {
        g_log_define->addListener(0xF1E231,[](const std::set<LogDefine>& old_value,
                    const std::set<LogDefine>& new_value){
            LYSLG_LOG_INFO(LYSLG_LOG_ROOT()) << "on_logger_conf_changed";
            // 新增
            for(auto& i:new_value){
                auto it = old_value.find(i);
                lyslg::Logger::ptr logger;
                if(it == old_value.end()) {
                    // 新增logger
                    logger = LYSLG_LOG_NAME(i.name);
                    // std::cout << "新增" << logger->getName()<< std::endl;
                }else {
                    if(!(i == *it)) {
                        // 修改
                        logger = LYSLG_LOG_NAME(i.name);
                        // std::cout << "修改" << logger->getName()<< std::endl;
                    }
                }
    
                logger->setLevel(i.level);
                if(!i.formatter.empty()) {
                    logger->setFormatter(i.formatter);
                    // std::cout << "i.formatter=" << i.formatter << std::endl;
                }

                logger->clearAppender();
                for(auto& a:i.appenders){
                    lyslg::LogAppender::ptr ap;
                    if(a.type == 1){
                        ap.reset(new FileLogAppender(a.file));
                    }else {
                        ap.reset(new StdoutLogAppender);
                    }
                    ap->setLevel(a.level);
                    if(!a.formatter.empty()) {
                        lyslg::LogFormatter::ptr formatter(new LogFormatter(a.formatter));
                        if(!formatter->isError()){
                            ap->setFormatter(formatter);
                        }else {
                            std::cout << "log name=" << i.name << " appender type=" << a.type
                                      << " formatter=" << a.formatter << " is invalid" << std::endl;
                        }
                    }

                    logger->addAppender(ap);
                }
                // std::cout << logger->toYamlString() << std::endl;
            }
            // 删除
            for(auto& i : old_value) {
                auto it = new_value.find(i);
                if(it == new_value.end()) {
                    // 删除logger
                    auto logger = LYSLG_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppender();
                }
            }
            // std::cout << LYSLG_LOG_NAME("root")->toYamlString() << std::endl;
            // std::cout << LYSLG_LOG_NAME("system")->toYamlString() << std::endl;
        });
    }
};

static LogIniter __log_init;

std::string LoggerManager::toYamlString() {
    YAML::Node node;
    for(auto& i:m_loggers){
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}


void LoggerManager::init(){

}

};