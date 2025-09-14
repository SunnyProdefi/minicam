#include "graph/inode.h"
#include "graph/isource.h"
#include "graph/node_factory.h"
#include "common/log.h"
#include <atomic>
#include <random>
#include <chrono>
#include <thread>

using namespace mc;

class SourceNode : public INode, public ISource
{
public:
    bool open(const NodeConfig& cfg) override
    {
        INode::open(cfg);
        return true;
    }

    void onRequest(const CaptureRequest& req, ResultCallback cb) override
    {
        int frames = 5;
        auto it = req.settings.ints.find("frames");
        if (it != req.settings.ints.end())
            frames = it->second;

        // 注册请求到总线
        if (auto b = bus())
        {
            std::lock_guard<std::mutex> lk(b->m);
            b->req[req.request_id] = {frames, std::move(cb)};
        }

        // 生成若干帧并进入处理队列（异步）
        int w = 640, h = 480;
        if (!req.streams.empty())
        {
            w = req.streams[0].width;
            h = req.streams[0].height;
        }

        for (int i = 0; i < frames; ++i)
        {
            Buffer buf;
            buf.width = w;
            buf.height = h;
            buf.stride = w * 4;
            buf.fmt = PixelFormat::RGBA8888;
            buf.ts = std::chrono::steady_clock::now();
            buf.frame_id = next_frame_id_++;
            buf.request_id = req.request_id;
            buf.data = std::make_shared<std::vector<uint8_t>>(buf.stride * buf.height);

            // 填充简单图案（随帧变化）
            std::fill(buf.data->begin(), buf.data->end(), static_cast<uint8_t>(32 + (i * 15) % 200));

            onBuffer(std::move(buf));
            // 简单节流，模拟硬件帧率
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void onBuffer(Buffer buf) override { enqueue(std::move(buf)); }

protected:
    void process(Buffer& b) override
    {
        // 模拟源节点前处理（此处无操作）
        LOG_INFO() << "SOURCE" << "[" << name() << "] processing buffer: req=" << b.request_id << " frame=" << b.frame_id;
        pushDown(std::move(b));
    }

private:
    std::atomic<int> next_frame_id_{0};
};

MC_REGISTER_NODE("Source", SourceNode);
