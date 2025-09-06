#pragma once
#include "../core/buffer.h"
#include "../core/queue.h"
#include "../common/noncopyable.h"
#include "pipeline_bus.h"
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <condition_variable>
#include "common/log.h"

namespace mc
{

    struct NodeConfig
    {
        std::string name;
        std::shared_ptr<PipelineBus> bus;
    };

    class INode : private NonCopyable
    {
    public:
        virtual ~INode() { stop(); }
        virtual bool open(const NodeConfig& cfg)
        {
            LOG_INFO() << "Node " << cfg.name << " opened";
            name_ = cfg.name;
            bus_ = cfg.bus;
            return true;
        }
        virtual void close() {}
        virtual void setDownstream(std::shared_ptr<INode> n)
        {
            LOG_INFO() << "Node " << name_ << " setting downstream to " << (n ? n->name() : "null");
            downstream_ = std::move(n);
        }

        // 上游调用入口
        virtual void onBuffer(Buffer buf) = 0;

        // 生命周期
        virtual void start()
        {
            LOG_INFO() << "Node " << name_ << " starting";
            running_ = true;
            th_ = std::thread([this] { runLoop(); });
        }
        virtual void stop()
        {
            running_ = false;
            cv_.notify_all();
            if (th_.joinable())
                th_.join();
        }

        const std::string& name() const { return name_; }

    protected:
        virtual void runLoop()
        {
            LOG_INFO() << "[" << name() << "] thread started";
            while (running_)
            {
                LOG_INFO() << "[" << name() << "] waiting for buffer...";
                auto opt = inq_.popBlocking(running_);
                if (!opt.has_value())
                    break;
                Buffer b = std::move(opt.value());
                LOG_INFO() << "[" << name() << "] dequeued buffer: req=" << b.request_id << " frame=" << b.frame_id;
                process(b);
                LOG_INFO() << "[" << name() << "] buffer processed";
            }
        }

        virtual void process(Buffer& b) = 0;

        void pushDown(Buffer b)
        {
            LOG_INFO() << "[" << name_ << "] pushing down buffer: req=" << b.request_id << " frame=" << b.frame_id;
            if (downstream_)
                downstream_->onBuffer(std::move(b));
        }
        void enqueue(Buffer b)
        {
            LOG_INFO() << "[" << name_ << "] enqueue buffer: req=" << b.request_id << " frame=" << b.frame_id;
            inq_.push(std::move(b));
            cv_.notify_one();
        }

        std::shared_ptr<PipelineBus> bus() const { return bus_; }

    private:
        std::string name_;
        std::shared_ptr<INode> downstream_;
        std::shared_ptr<PipelineBus> bus_;
        std::thread th_;
        std::atomic<bool> running_{false};
        BlockingQueue<Buffer> inq_;
        std::condition_variable cv_;
    };

}  // namespace mc
