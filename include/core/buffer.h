#pragma once
#include "types.h"
#include "../common/noncopyable.h"
#include <memory>
#include <vector>
#include <atomic>

namespace mc
{

    struct Buffer
    {
        std::shared_ptr<std::vector<uint8_t>> data;
        int width{0}, height{0}, stride{0};
        PixelFormat fmt{PixelFormat::RGBA8888};
        TimePoint ts{};
        int frame_id{-1};
        int request_id{-1};  // 透传请求ID
    };

    class BufferPool : private NonCopyable
    {
    public:
        explicit BufferPool(size_t cap, size_t bytes_per_buf) : cap_(cap), bytes_(bytes_per_buf) {}

        std::shared_ptr<std::vector<uint8_t>> acquire()
        {
            // 简化实现：直接新建。进阶可维护空闲队列复用。
            return std::make_shared<std::vector<uint8_t>>(bytes_);
        }

    private:
        size_t cap_, bytes_;
    };

}  // namespace mc
