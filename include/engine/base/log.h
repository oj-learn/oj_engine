#pragma once

#include "spdlog/fmt/fmt.h"
#include <source_location>
#include <string>
//#include "fmt/format.h"
// #include <experimental/source_location>

/*---------------------------------------------------------------------------------
基本色碼表，高位元16進位表示底色，低位元16進位表示字色，組合範圍0~255 (參考:http://blog.wildsky.cc/posts/c_code_note/)
---------------------------------------------------------------------------------*/
enum {
    SCREEN_COLOR_BLACK         = 0,   // 黑
    SCREEN_COLOR_BULE          = 1,   // 藍
    SCREEN_COLOR_GREEN         = 2,   // 綠
    SCREEN_COLOR_CYAN          = 3,   // 青
    SCREEN_COLOR_RED           = 4,   // 紅
    SCREEN_COLOR_PURPLE        = 5,   // 紫
    SCREEN_COLOR_YELLOW        = 6,   // 黃
    SCREEN_COLOR_WHITE         = 7,   // 白
    SCREEN_COLOR_BRIGHT_BLACK  = 8,   // 亮黑
    SCREEN_COLOR_BRIGHT_BULE   = 9,   // 亮藍
    SCREEN_COLOR_BRIGHT_GREEN  = 10,  // 亮綠
    SCREEN_COLOR_BRIGHT_CYAN   = 11,  // 亮青
    SCREEN_COLOR_BRIGHT_RED    = 12,  // 亮紅
    SCREEN_COLOR_BRIGHT_PURPLE = 13,  // 亮紫
    SCREEN_COLOR_BRIGHT_YELLOW = 14,  // 亮黃
    SCREEN_COLOR_BRIGHT_WHITE  = 15,  // 亮白
    // 加底色色碼表
    SCREEN_COLOR_BULE_AND_BRIGHT_PURPLE  = 29,   // 藍底亮紫字
    SCREEN_COLOR_GREEN_AND_BRIGHT_YELLOW = 46,   // 綠底亮黃字
    SCREEN_COLOR_YELLOW_AND_GREEN        = 226,  // 黃底綠字
    SCREEN_COLOR_YELLOW_AND_BRIGHT_RED   = 236,  // 黃底亮紅字
    SCREEN_COLOR_GREEN_AND_BRIGHT_WHITE  = 63,   // 綠底底亮白字
    SCREEN_COLOR_RED_AND_BRIGHT_WHITE    = 207,  // 紅底底亮白字
};


/*---------------------------------------------------------------------------------
log_t

(參考:https://github.com/gabime/spdlog/wiki/1.-QuickStart)

sink	: 对最终输出的抽象，例如文件，控制台...
logger	: 记录器，需要绑定一个或多个sink，在每次日志调用时会选择正确的sink

#include "spdlog/sinks/basic_file_sink.h"
    // Create basic file logger (not rotated)
    auto my_logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");

#include "spdlog/sinks/rotating_file_sink.h"
    // Create a file rotating logger with 5mb size max and 3 rotated files
    auto rotating_logger = spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);

#include "spdlog/sinks/daily_file_sink.h"
    // Create a daily logger - a new file is created every day on 2:30am
    auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);

#include "spdlog/async.h"
    // default thread pool settings can be modified *before* creating the async logger:
    // spdlog::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
    auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");
    // alternatively:
    // auto async_file = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("async_file_logger", "logs/async_log.txt");
---------------------------------------------------------------------------------*/
// struct config_logger_t;
class log_t {
public:
    int m_writeLevel;

public:
    //-----------------------------------------------------------------------------
    static log_t& singletion();

public:
    //-----------------------------------------------------------------------------
    void init(std::string path);
    void close();
    //-----------------------------------------------------------------------------
    void levelSet(const std::string level);
    //-----------------------------------------------------------------------------
    // void registLogger(config_logger_t& cfg, bool default_ = false);
    //-----------------------------------------------------------------------------
    int write(int logLevel, std::string&& data, const std::source_location location = std::source_location::current());
    //-----------------------------------------------------------------------------
    bool printByColor(int Color, const std::string& log);
};

/*---------------------------------------------------------------------------------
统一用这些宏实现打印信息到屏幕或记录到文件。printf时不需要再去写 %d,%u,这种东西
示例:logTrace("这是一个示例",nnn, DEF_DumpVar(xxx));
##__VA_ARGS__: 只有前面还有参数才能加 ## 这个符号
#var: 展开为　"var"
---------------------------------------------------------------------------------*/
#define logSingletion                   log_t::singletion()
#define logFormat(...)                  fmt::format(__VA_ARGS__)
#define logAlign(...)                   logFormat("{0:-^{1}}", logFormat(__VA_ARGS__), 80)
#define logWrite(logLevel, fmstr, ...)  (logLevel >= logSingletion.m_writeLevel ? logSingletion.write(logLevel, logFormat(fmstr, ##__VA_ARGS__)) : 0)
#define logTrace(fmstr, ...)            logWrite(0, fmstr, ##__VA_ARGS__)
#define logDebug(fmstr, ...)            logWrite(1, fmstr, ##__VA_ARGS__)
#define logInfo(fmstr, ...)             logWrite(2, fmstr, ##__VA_ARGS__)
#define logError(fmstr, ...)            logWrite(4, fmstr, ##__VA_ARGS__)
#define logErrorReturn(ret, fmstr, ...) return logError(fmstr, ##__VA_ARGS__), ret
