#pragma once
#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace mc
{

    enum class LogLevel
    {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR
    };

    class Logger
    {
    public:
        static Logger& instance();

        // 初始化：在 main() 一次性调用
        //   log_dir: 日志目录（默认 logs）
        //   program_name: 前缀（默认 minicamera）
        //   console_color: 控制台是否彩色
        void init(const std::string& log_dir = "logs", const std::string& program_name = "minicam", bool console_color = true);

        // 运行时调整最小等级（默认 DEBUG）
        void setLevel(LogLevel lv) { min_level_.store(lv, std::memory_order_relaxed); }

        // 真正写入（供 RAII 行对象调用）
        void write(LogLevel lv, const char* file, int line, const std::string& text);

        // 获取当前最小等级
        LogLevel level() const { return min_level_.load(std::memory_order_relaxed); }

        // 返回当前日志文件路径（便于调试/提示）
        std::string logfile_path() const { return logfile_path_; }

    private:
        Logger() = default;
        ~Logger();

        std::string make_ts_filename(const std::string& dir, const std::string& prefix);
        static std::string now_time_string();           // YYYY-MM-DD HH:MM:SS
        static std::string now_compact_string();        // YYYYMMDD-HHMMSS
        static const char* level_to_cstr(LogLevel lv);  // "DEBUG"/...

        // 控制台颜色
        static const char* level_color(LogLevel lv);  // ANSI color prefix
        static const char* color_reset();             // ANSI reset

    private:
        std::ofstream ofs_;
        std::string logfile_path_{};
        std::atomic<LogLevel> min_level_{LogLevel::DEBUG};
        bool console_color_{true};
        std::mutex mtx_;
    };

    // RAII：一条日志行的构造器，支持流式 <<
    class LogLine
    {
    public:
        LogLine(LogLevel lv, const char* file, int line) : lv_(lv), file_(file), line_(line) {}
        ~LogLine() { Logger::instance().write(lv_, file_, line_, oss_.str()); }

        template <typename T>
        LogLine& operator<<(const T& v)
        {
            oss_ << v;
            return *this;
        }

    private:
        LogLevel lv_;
        const char* file_;
        int line_;
        std::ostringstream oss_;
    };

}  // namespace mc
