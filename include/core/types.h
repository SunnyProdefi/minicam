#pragma once
#include <cstdint>
#include <chrono>
#include <string>

namespace mc
{
    using TimePoint = std::chrono::steady_clock::time_point;

    enum class PixelFormat
    {
        RGBA8888,
        GRAY8,
        RAW8
    };

    enum class Error
    {
        OK = 0,
        INVALID,
        CLOSED,
        TIMEOUT,
        UNKNOWN
    };
}  // namespace mc
