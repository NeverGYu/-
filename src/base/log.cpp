#include "log.h"
#include "config.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <functional>
#include <chrono>
#include <sstream>

namespace sylar
{


/*-------------  公共函数 ----------------*/

/**
* @brief 模板函数
* @details 用来绑定对应的string <--> function<FormatItem::ptr>
*/
template<typename T>
LogFormatter::FormateItem::ptr CreateFormateItem(const std::string& fmt) { return std::make_shared<T>(fmt); };



/*-------------  LogLevel  ----------------*/
inline
const char* LogLevel::LevelToString(LogLevel::Level level)
{
    switch (level)
    {
    case FATAL:
        return "FATAL";
        break;
    case ALERT:
        return "ALERT";
        break;
    case CRIT:
        return "CRIT";
        break;
    case ERROR:
        return "ERROR";
        break;
    case WARN:
        return "WARN";
        break;
    case NOTICE:
        return "NOTICE";
        break;
    case INFO:
        return "INFO";
        break;
    case DEBUG:
        return "DEBUG";
        break;
    default:
        return "NOTSET";
        break;
    }
}

inline
const LogLevel::Level LogLevel::StringToLevel(const std::string& str)
{
    if (str == "fatal")   return LogLevel::FATAL;
    if (str == "alert")   return LogLevel::ALERT;
    if (str == "crit")    return LogLevel::CRIT;
    if (str == "error")   return LogLevel::ERROR;
    if (str == "warn")    return LogLevel::WARN;
    if (str == "notice")  return LogLevel::NOTICE;
    if (str == "info")    return LogLevel::INFO;
    if (str == "debug")   return LogLevel::DEBUG;

    if (str == "FATAL")   return LogLevel::FATAL;
    if (str == "ALERT")   return LogLevel::ALERT;
    if (str == "CRIT")    return LogLevel::CRIT;
    if (str == "ERROR")   return LogLevel::ERROR;
    if (str == "WARN")    return LogLevel::WARN;
    if (str == "NOTICE")  return LogLevel::NOTICE;
    if (str == "INFO")    return LogLevel::INFO;
    if (str == "DEBUG")   return LogLevel::DEBUG;

    return LogLevel::NOTSET;
    
}

/*-------------  Logger  ----------------*/
Logger::Logger(const std::string& name)
    : m_name(name)
    , m_level(LogLevel::DEBUG)
    , m_createTime(GetElapsedMS())
{}

void Logger::addAppender(LogAppender::ptr appender)
{
    MutexType::Lock lock(m_mutex);
    if (appender)
    {
        m_appenders.push_back(appender);
    }
    
}

void Logger::deleteAppender(LogAppender::ptr appender)
{
    MutexType::Lock lock(m_mutex);
    for(auto it = m_appenders.begin(); it != m_appenders.end(); it++) {
        if(*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders()
{
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void Logger::log(LogEvent::ptr event)
{
    if (event->getLevel() <= m_level)
    {
        for (auto& i : m_appenders)
        {
            // 将日志写入到每个日志输出地：控制台 or 文件形式
            i->log(event);
        }
    }
    
}

std::string Logger::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    node["level"] = LogLevel::LevelToString(m_level);
    for(auto &i : m_appenders) {
        if (i) 
        {  // 检查 i 是否为 null
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        } 
        else {
            // 记录或处理空指针情况
            std::cerr << "Appender is null!" << std::endl;
        }
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}


/*-------------  LoggerAppender  ----------------*/

LogAppender::LogAppender(LogFormatter::ptr formatter)
    : defalut_formatter(formatter)
{}

void LogAppender::setLogFormatter(LogFormatter::ptr val)
{
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
}

LogFormatter::ptr LogAppender::getLogFormatter()
{
    MutexType::Lock lock(m_mutex);
    return m_formatter ? m_formatter : defalut_formatter;
}



/*-------------  StdoutLogAppender  ----------------*/

StdoutLogAppender::StdoutLogAppender()
    : LogAppender(LogFormatter::ptr(new LogFormatter))
{}

void StdoutLogAppender::log(LogEvent::ptr event)
{
    MutexType::Lock lock(m_mutex);
    if (m_formatter)
    {
        // 如果父类的成员日志格式化器被指定过
        m_formatter->format(std::cout, event);
    }
    else
    {
        // 调用默认的日志格式化器
        defalut_formatter->format(std::cout, event);
    }
    
}

std::string StdoutLogAppender::toYamlString()
{
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    node["pattern"] = m_formatter ? m_formatter->getPattern() : defalut_formatter->getPattern();
    std::stringstream ss;
    ss << node;
    return ss.str();
}


/*-------------  FileLogAppender  ----------------*/

FileAppender::FileAppender(const std::string& file)
    : LogAppender(LogFormatter::ptr(new LogFormatter))
    , m_filePath(file)
{
    reopen();
    if (m_reopen_error)
    {
        std::cout << "reopen file " << m_filePath << " error" << std::endl;
    }
    
}

bool FileAppender::reopen()
{
    MutexType::Lock lock(m_mutex);
    if(m_fileStream) {
        m_fileStream.close();
    }
    m_fileStream.open(m_filePath, std::ios::app);
    m_reopen_error = !m_fileStream;
    return !m_reopen_error;
}

/**
 * @brief 如果一个日志事件距离上次写日志超过3秒，那就重新打开一次日志文件
 */
void FileAppender::log(LogEvent::ptr event)
{
    uint64_t now = event->getTime();
    if (now >= m_lastTime + 3)
    {
        reopen();
        if (m_reopen_error)
        {
            std::cout << "reopen file" << m_filePath << "error" <<std::endl;
        }
        m_lastTime = now;
    }
    if (m_reopen_error)
    {
        return;
    }
    MutexType::Lock lock(m_mutex);
    if (m_formatter)
    {
        if (!m_formatter->format(m_fileStream, event))
        {
            std::cout << "[ERROR] FileLogAppender::log() format error" << std::endl;
        }
    }
    else 
    {
        if(!defalut_formatter->format(m_fileStream, event))
        {
            std::cout << "[ERROR] FileLogAppender::log() format error" << std::endl;
        }
    }
}

/**
 * @brief 
 */
std::string FileAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["pattern"] = m_formatter ? m_formatter->getPattern() : defalut_formatter->getPattern();
    node["file"] = m_filePath;
    std::stringstream ss;
    ss << node;
    return ss.str();
}

/*-------------  FormatItem的子类 ----------------*/
class MessageFormatItem : public LogFormatter::FormateItem
{
public:
    MessageFormatItem(const std::string &str) {}
    void format(std::ostream& os, LogEvent::ptr event)  override
    {
        os << event->getContent();
    }
}; 

class LevelFormatItem : public LogFormatter::FormateItem {
public:
    LevelFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << LogLevel::LevelToString(event->getLevel());
    }
};

class ElapseFormatItem : public LogFormatter::FormateItem {
public:
    ElapseFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};
    
class LoggerNameFormatItem : public LogFormatter::FormateItem {
public:
    LoggerNameFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << event->getLoggerName();
    }
};
    
class ThreadIdFormatItem : public LogFormatter::FormateItem {
public:
    ThreadIdFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << event->getThreadid();
    }
};
    
class FiberIdFormatItem : public LogFormatter::FormateItem {
public:
    FiberIdFormatItem(const std::string &str){}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << event->getFiberid();
    }
};
    
class ThreadNameFormatItem : public LogFormatter::FormateItem {
public:
    ThreadNameFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};
    
class DateTimeFormatItem : public LogFormatter::FormateItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        : m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    
    void format(std::ostream& os, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
        std::string m_format;
};
    
class FileNameFormatItem : public LogFormatter::FormateItem {
public:
    FileNameFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << event->getFileName();
    }
};
    
class LineFormatItem : public LogFormatter::FormateItem {
public:
    LineFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << event->getLine();
    }
};
    
class NewLineFormatItem : public LogFormatter::FormateItem {
public:
    NewLineFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << std::endl;
    }
};
    
class StringFormatItem : public LogFormatter::FormateItem {
public:
    StringFormatItem(const std::string& str)
        : m_string(str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};
    
class TabFormatItem : public LogFormatter::FormateItem {
public:
    TabFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << "\t";
    }
};
    
class PercentSignFormatItem : public LogFormatter::FormateItem {
public:
    PercentSignFormatItem(const std::string &str) {}
    void format(std::ostream &os, LogEvent::ptr event) override {
        os << "%";
    }
};



/*-------------  LoggerFormatter  ----------------*/
LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern)
{
    init();
}

void LogFormatter::init()
{
    /**
    * @brief
    * 简单的状态机判断，提取pattern中的常规字符和模式字符
    * 解析的过程就是从头到尾遍历，根据状态标志决定当前字符是常规字符还是模式字符
    * 一共有两种状态，即正在解析常规字符和正在解析模板转义字符
    */

    // 按顺序存储从m_pattern解析到的patterns
    std::vector<std::pair<int,std::string>> patterns;
    // 判断解析过程是否出错
    bool error = false;
    // 临时存储常规字符串
    std::string temp;
    // 存储日期格式的字符串%d{}
    std::string date;
    // 判断是否在解析常规字符串
    bool parse_normal = true;
    // 从m_pattern的起始字符串开始
    size_t i = 0;
    while (i < m_pattern.size())
    {
        std::string c = std::string(1, m_pattern[i]);
        // 首先判断是否是%
        if (c == "%")
        {
            if (parse_normal)   // 这表示前面一个字符不是%，仍然是普通字符
            {
                patterns.push_back(std::make_pair(0, temp));
                temp.clear();
                parse_normal = false;
                ++i;
                continue;
            }
            else
            {
                // 这说明%是转义字符，直接将其添加到patterns
                patterns.push_back(std::make_pair(1,"%"));
                parse_normal = true;
                ++i;
                continue;
            }
        }
        else    // 这表示当前字符不是%
        {
            if (parse_normal)   // 这表示前面一个字符不是%，仍然是普通字符
            {
                temp += c;
                ++i;
                continue;
            }

            else
            {
                // 这表示是模板字符
                patterns.push_back(std::make_pair(1,c));
                parse_normal = true;

                if (c != "d")  // 这说明不是日期格式
                {
                    ++i;
                    continue;
                }
                else
                {
                    // 这表明是日期格式
                    ++i;
                    if (i < m_pattern.size() && m_pattern[i] != '{')
                    {
                        // 说明格式错误
                        continue;
                    }
                    else
                    {
                        while (i < m_pattern.size() && m_pattern[i] != '}')
                        {
                            date.push_back(m_pattern[i]);
                            ++i;
                        }
                    }
                    if (m_pattern[i] != '}')
                    {
                        // %d后面的大括号没有闭合，直接报错
                        std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] '{' not closed" << std::endl;
                        error = true;
                        break;
                    }
                    ++i;
                    continue;
                }
            }
        }  
    }

    // 判断是否出错
    if (error)
    {
        m_error = error;
        return;
    }
    // 循环结束完，需要将剩余的字符串也添加到patterns中
    if (!temp.empty())
    {
        patterns.push_back(std::make_pair(0,temp));
        temp.clear();
    }

    // 定义具体的日志处理类，根据日志模板来调用对应的日志处理函数
    static std::map<std::string, std::function<FormateItem::ptr(const std::string&)>> s_format_item =
    {
        {"m", CreateFormateItem<MessageFormatItem>},
        {"p", CreateFormateItem<LevelFormatItem>},
        {"c", CreateFormateItem<LoggerNameFormatItem>},
        {"r", CreateFormateItem<ElapseFormatItem>},
        {"f", CreateFormateItem<FileNameFormatItem>},
        {"l", CreateFormateItem<LineFormatItem>},
        {"t", CreateFormateItem<ThreadIdFormatItem>},
        {"F", CreateFormateItem<FiberIdFormatItem>},
        {"N", CreateFormateItem<ThreadNameFormatItem>},
        {"%", CreateFormateItem<PercentSignFormatItem>},
        {"T", CreateFormateItem<TabFormatItem>},
        {"n", CreateFormateItem<NewLineFormatItem>}
    };

    for (auto iterator : patterns)
    {
        if (iterator.first == 0)
        {
            // 这是在处理普通的常规字符
            m_items.push_back(FormateItem::ptr(new StringFormatItem(iterator.second)));
        }
        else if (iterator.second == "d")
        {
            // 这是在处理日志中的日期
            m_items.push_back(FormateItem::ptr(new DateTimeFormatItem(date)));
        }
        else
        {
            // 这是在处理模板字符
            if (auto it = s_format_item.find(iterator.second); it !=  s_format_item.end())
            {
                // 这说明能够找到对应模板字符的类
                m_items.push_back(it->second(iterator.second));
            }
            else
            {
                // 这表示该字符不是模板字符
                std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] " << "unknown format item: " << iterator.second << std::endl;
                error = true;
                break;
            }
        }
    }

    if(error) {
        m_error = true;
        return;
    }
}

std::ostream& LogFormatter::format(std::ostream& os, LogEvent::ptr event)
{
    //  遍历具体的日志处理类
    for (auto& i : m_items)
    {
        i->format(os, event);
    }
    return os;
}

std::string LogFormatter::format(LogEvent::ptr event)
{
    std::stringstream ss;
    //  遍历具体的日志处理类
    for (auto& i: m_items)
    {
        i->format(ss, event);
    }
    return ss.str();
}

LoggerManager::LoggerManager()
{
    m_root.reset(new Logger("root"));
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers[m_root->getLoggerName()] = m_root;
    init();
}

/**
 * @todo 实现从配置文件加载日志配置
 */
void LoggerManager::init() 
{
    
}

std::string LoggerManager::toYamlString()
{
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for (auto& i : m_loggers)
    {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    MutexType::Lock lock(m_mutex);
    
    auto it = m_loggers.find(name);
    if (it != m_loggers.end())
    {
        return it->second;
    }
    Logger::ptr logger(new Logger(name));
    logger->addAppender(LogAppender::ptr(new StdoutLogAppender));
    m_loggers[name] = logger;
    return logger;
}


/*------------------------从配置文件中加载日志配置-------------------------------*/
/**
 * @brief 日志输出器配置结构体定义
 */
struct LogAppenderDefine
{
    using LogAppenderType = int;
    LogAppenderType type = 0; // 0 代表Stdout , 1 代表File
    std::string pattern;      // 代表日志使用的模板
    std::string filepath;

    bool operator==(const LogAppenderDefine& rhs) const
    {
        return type == rhs.type && pattern == rhs.pattern && filepath == rhs.filepath;
    }
};

/**
 * @brief 日志器配置结构体定义
 */

struct LoggerDefine
{
    std::string name;
    LogLevel::Level level = LogLevel::NOTSET;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LoggerDefine& rhs) const
    {
        return name == rhs.name && level== rhs.level && appenders == rhs.appenders;
    }

    bool operator<(const LoggerDefine& rhs) const
    {
        return name < rhs.name;
    }

    bool isVaild() const { return !name.empty(); }
};

template<>
class lexicalCast<std::string, LoggerDefine>
{
public:
    LoggerDefine operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        LoggerDefine ld;
        if (!node["name"].IsDefined())
        {
            // 这说明给定的字符串中没有name参数
            std::cout << "log config error: name is null, " << node << std::endl;
            throw std::logic_error("log config name is null");
        }
        ld.name = node["name"].as<std::string>();
        ld.level =  LogLevel::StringToLevel(node["level"].IsDefined() ? node["level"].as<std::string>() : "");

        if (!node["appenders"].IsDefined())
        {
            for (size_t i = 0; i < node["appenders"].size(); ++i)
            {
                auto a = node["appenders"][i];
                // 定义appender的结构体
                LogAppenderDefine lad;
                if (!a["type"].IsDefined())
                {
                    // 这说明给定的字符串中没有type参数
                    std::cout << "logAppender config error: type is null, " << node << std::endl;
                    throw std::logic_error("logAppender config type is null");
                }

                std::string type = a["type"].as<std::string>();
                // 接下来判断是哪种Appender
                if (type == "FileAppender")      // fileAppender
                {
                    lad.type = 1;
                    if (!a["file"].IsDefined())
                    {
                        // 这说明没有文件路径
                        std::cout << "logAppender config error: file is null, " << node << std::endl;
                        throw std::logic_error("logAppender config file is null");
                    }
                    lad.filepath = a["file"].as<std::string>();
                    if (!a["pattern"].IsDefined())
                    {
                        // 这说明没有文件路径
                        std::cout << "logAppender config error: pattern is null, " << node << std::endl;
                        throw std::logic_error("logAppender config pattern is null");
                    }
                    lad.pattern = a["pattern"].as<std::string>();
                }
                else if (type == "StdoutAppender")
                {
                    lad.type = 0;
                    if (!a["pattern"].IsDefined())
                    {
                        // 这说明没有文件路径
                        std::cout << "logAppender config error: pattern is null, " << node << std::endl;
                        throw std::logic_error("logAppender config pattern is null");
                    }
                    lad.pattern = a["pattern"].as<std::string>();
                }
                else
                {
                    std::cout << "log appender config error: appender type is invalid, " << a << std::endl;
                    continue;
                }
                ld.appenders.push_back(lad);
            }
        }
        return ld;
    }
};

template<>
class lexicalCast<LoggerDefine, std::string>
{
public:
    std::string operator()(LoggerDefine ld)
    {
        YAML::Node node;
        node["name"] = ld.name;
        node["level"] = LogLevel::LevelToString(ld.level);
        for (auto& i : ld.appenders)
        {
            YAML::Node na;
            if (i.type == 1)    // 这是个fileAppender
            {
                na["type"] = "FileAppender";
                na["file"] = i.filepath;
            }
            else if (i.type == 0)
            {
                na["type"] = "StdoutLogAppender";
            }
            
            if(!i.pattern.empty())
            {
                na["pattern"] = i.pattern;
            }
            node["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

sylar::ConfigVar<std::set<LoggerDefine>>::ptr g_log_defines = 
    sylar::Config::Lookup("logs", "logs config", std::set<LoggerDefine>());

class LogIniter
{
public:
    /**
     *  @brief 构造函数 
     */
    LogIniter()
    {
        g_log_defines->addlistener([](const std::set<LoggerDefine>& oldValue, const std::set<LoggerDefine>& newValue){
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on log config change";
            for (auto &i : newValue)
            {
                auto it = oldValue.find(i);
                sylar::Logger::ptr logger;
                if (it == oldValue.end())
                {
                    // 这说明该日志是新的，不存在与原来的旧日志
                    logger = SYLAR_LOG_NAME(i.name);
                }
                else
                {
                    // i 是新日志，而 it 代表在旧日志中找到了这个新日志。(通过loggerDefine.name，loggerDefine.level，loggerDefine.appenders)
                    if (!(i == *it))
                    {
                        // 这说明新旧日志的成员发生了变化
                        logger = SYLAR_LOG_NAME(i.name);
                    }
                    else
                    {
                        // 这说明新旧日志的成员没有发生变化
                        continue;
                    }
                }
                // 修改logger日志器
                logger->setLoggerLevel(i.level);
                logger->clearAppenders();
                for (auto& a : i.appenders)
                {
                    sylar::LogAppender::ptr ap;
                    if (a.type == 1)
                    {
                        ap.reset(new FileAppender(a.filepath));
                    }
                    else if (a.type == 2)
                    {
                        ap.reset(new StdoutLogAppender);
                    }
                    else 
                    {
                        continue;
                    }

                    if(!a.pattern.empty())
                    {
                        ap->setLogFormatter(LogFormatter::ptr(new LogFormatter(a.pattern)));
                    } 
                    else
                    {
                        ap->setLogFormatter(LogFormatter::ptr(new LogFormatter));
                    }
                    logger->addAppender(ap);
                }
            }
             // 以配置文件为主，如果程序里定义了配置文件中未定义的logger，那么把程序里定义的logger设置成无效
             for(auto &i : oldValue) 
             {
                auto it = newValue.find(i);
                if(it == newValue.end()) 
                {
                    auto logger = SYLAR_LOG_NAME(i.name);
                    logger->setLoggerLevel(LogLevel::NOTSET);
                    logger->clearAppenders();
                }
            }
        });
    }
};

/**
 * @brief   在main函数之前注册配置更改的回调函数，用于在更新配置时将log相关的配置加载到Config
 */
static LogIniter __log_init;

}

