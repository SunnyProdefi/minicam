#pragma once
#include <mutex>
#include <condition_variable>
#include <deque>
#include <optional>
#include "common/log.h"

template <typename T>
class BlockingQueue
{
public:
    void push(T v)
    {
        LOG_INFO() << "BlockingQueue push enter";
        {
            std::lock_guard<std::mutex> lk(m_);
            q_.push_back(std::move(v));
        }
        cv_.notify_one();
    }

    std::optional<T> popBlocking(std::atomic_bool& stop_flag)
    {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&] { return stop_flag || !q_.empty(); });
        if (stop_flag && q_.empty())
            return std::nullopt;
        T v = std::move(q_.front());
        q_.pop_front();
        return v;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lk(m_);
        q_.clear();
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::deque<T> q_;
};
