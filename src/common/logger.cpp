#include "common/logger.h"
#include <filesystem>

namespace fs = std::filesystem;
namespace mc
{

    Logger& Logger::instance()
    {
        static Logger inst;
        return inst;
    }

    void Logger::init(const std::string& log_dir, const std::string& program_name, bool console_color)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        console_color_ = console_color;

        // 解析日志目录：相对路径统一相对工程根目录
        fs::path root = fs::current_path();
        while (!fs::exists(root / "CMakeLists.txt") && root.has_parent_path())
        {
            root = root.parent_path();
        }

        fs::path dir = log_dir;
        if (dir.empty())
        {
            dir = root;
        }
        else if (!dir.is_absolute())
        {
            dir = root / dir;
        }

        // 创建目录
        if (!dir.empty())
        {
            fs::create_directories(dir);
        }

        logfile_path_ = make_ts_filename(dir.string(), program_name);
        ofs_.open(logfile_path_, std::ios::out | std::ios::app);
        if (!ofs_)
        {
            // 打不开文件就仅输出到控制台
            std::cerr << "[LOGGER] failed to open log file: " << logfile_path_ << std::endl;
            logfile_path_.clear();
        }
        else
        {
            ofs_ << "==== Log start at " << now_time_string() << " ====" << std::endl;
            ofs_.flush();
        }
    }

    Logger::~Logger()
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (ofs_)
        {
            ofs_ << "==== Log end at " << now_time_string() << " ====" << std::endl;
            ofs_.flush();
            ofs_.close();
        }
    }

    void Logger::write(LogLevel lv, const char* file, int line, const std::string& text)
    {
        if (lv < level())
            return;

        const std::string ts = now_time_string();

        std::ostringstream linebuf;
        linebuf << "[" << ts << "] "
                << "[" << level_to_cstr(lv) << "] " << file << ":" << line << " | " << text;

        const std::string final_line = linebuf.str();

        std::lock_guard<std::mutex> lk(mtx_);

        // 控制台
        if (console_color_)
        {
            std::cout << level_color(lv) << final_line << color_reset() << "\n";
        }
        else
        {
            std::cout << final_line << "\n";
        }
        std::cout.flush();

        // 文件
        if (ofs_)
        {
            ofs_ << final_line << "\n";
            ofs_.flush();
        }
    }

    std::string Logger::make_ts_filename(const std::string& dir, const std::string& prefix)
    {
        std::ostringstream oss;
        oss << dir << "/" << prefix << "-" << now_compact_string() << ".log";
        return oss.str();
    }

    std::string Logger::now_time_string()
    {
        using clock = std::chrono::system_clock;
        auto t = clock::to_time_t(clock::now());
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string Logger::now_compact_string()
    {
        using clock = std::chrono::system_clock;
        auto t = clock::to_time_t(clock::now());
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y%m%d-%H%M%S");
        return oss.str();
    }

    const char* Logger::level_to_cstr(LogLevel lv)
    {
        switch (lv)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNK";
        }
    }

    const char* Logger::level_color(LogLevel lv)
    {
        // ANSI 颜色，仅控制台用；文件不带颜色
        switch (lv)
        {
        case LogLevel::DEBUG:
            return "\033[37m";  // 灰
        case LogLevel::INFO:
            return "\033[32m";  // 绿
        case LogLevel::WARN:
            return "\033[33m";  // 黄
        case LogLevel::ERROR:
            return "\033[31m";  // 红
        default:
            return "\033[0m";
        }
    }
    const char* Logger::color_reset() { return "\033[0m"; }

}  // namespace mc
