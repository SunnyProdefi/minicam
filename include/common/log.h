#pragma once
#include "logger.h"

// 方便在任意 .cpp 中直接用
#define LOG_DEBUG() mc::LogLine(mc::LogLevel::DEBUG, __FILE__, __LINE__)
#define LOG_INFO() mc::LogLine(mc::LogLevel::INFO, __FILE__, __LINE__)
#define LOG_WARN() mc::LogLine(mc::LogLevel::WARN, __FILE__, __LINE__)
#define LOG_ERROR() mc::LogLine(mc::LogLevel::ERROR, __FILE__, __LINE__)
