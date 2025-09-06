#pragma once
#include <mutex>
#include <condition_variable>
#include <deque>
#include <optional>
#include <atomic>
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
        LOG_INFO() << "BlockingQueue push exit";
        cv_.notify_one();
    }

    std::optional<T> popBlocking(std::atomic_bool& running_flag)
    {
        LOG_INFO() << "BlockingQueue pop enter";
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&] { return !running_flag || !q_.empty(); });
        if (!running_flag && q_.empty())

        {
            LOG_INFO() << "BlockingQueue pop stop";
            return std::nullopt;
        }
        T v = std::move(q_.front());
        q_.pop_front();
        LOG_INFO() << "BlockingQueue pop exit";
        return v;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lk(m_);
        q_.clear();
    }

    void notify_all()
    {
        cv_.notify_all();
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::deque<T> q_;
};
