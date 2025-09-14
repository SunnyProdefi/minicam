#include "service/camera_service.h"
#include "common/log.h"
#include <memory>

using namespace mc;

bool CameraService::openSession()
{
    std::lock_guard<std::mutex> lk(m_);
    if (opened_)
        return true;

    bus_ = std::make_shared<PipelineBus>();

    auto source = NodeFactory::instance().create("Source");
    if (source == nullptr)
    {
        LOG_ERROR() << "create source node failed";
        return false;
    }
    auto pack = NodeFactory::instance().create("Pack");
    if (pack == nullptr)
    {
        LOG_ERROR() << "create pack node failed";
        return false;
    }
    auto sink = NodeFactory::instance().create("Sink");
    if (sink == nullptr)
    {
        LOG_ERROR() << "create sink node failed";
        return false;
    }
    if (!source || !pack || !sink)
    {
        LOG_ERROR() << "create nodes failed";
        return false;
    }

    source->open({"source", bus_});
    pack->open({"pack", bus_});
    sink->open({"sink", bus_});

    exec_.addNode(source);
    exec_.addNode(pack);
    exec_.addNode(sink);
    exec_.link(source, pack);
    exec_.link(pack, sink);
    exec_.start();

    LOG_INFO() << "Camera session opened";

    source_node_ = source;
    source_iface_ = dynamic_cast<ISource*>(source_node_.get());
    if (!source_iface_)
    {
        LOG_ERROR() << "Source node does not implement ISource";
        exec_.stop();
        source_node_.reset();
        return false;
    }

    opened_ = true;
    return true;
}

void CameraService::closeSession()
{
    std::lock_guard<std::mutex> lk(m_);
    if (!opened_)
        return;
    exec_.stop();
    source_node_.reset();
    source_iface_ = nullptr;
    bus_.reset();
    opened_ = false;
}

std::future<CaptureResult> CameraService::submit(const CaptureRequest& req, ResultCallback cb)
{
    std::promise<CaptureResult> prom;
    auto fut = prom.get_future();

    // 将 promise 移入 shared_ptr，便于在 std::function 里按值捕获（可拷贝）
    auto p = std::make_shared<std::promise<CaptureResult>>(std::move(prom));

    ResultCallback wrapped = [p, cb](const CaptureResult& r) mutable
    {
        if (cb)
            cb(r);
        // 可能被多次触发：第二次 set_value 会抛异常，做保护
        try
        {
            p->set_value(r);
        }
        catch (const std::future_error&)
        {
            // 已经设置过就忽略
        }
    };

    if (!opened_ || !source_iface_)
    {
        LOG_INFO() << "service not opened, reject request " << req.request_id;
        CaptureResult r;
        r.request_id = req.request_id;
        r.status = Error::CLOSED;
        wrapped(r);
        return fut;
    }

    source_iface_->onRequest(req, wrapped);
    return fut;
}
