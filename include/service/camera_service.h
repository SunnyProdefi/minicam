#pragma once
#include "../core/request.h"
#include "../graph/executor.h"
#include "../graph/node_factory.h"
#include "../graph/isource.h"
#include "../graph/pipeline_bus.h"
#include "../common/log.h"
#include <mutex>
#include <future>

namespace mc
{

    class CameraService
    {
    public:
        static CameraService& instance()
        {
            static CameraService s;
            return s;
        }

        bool openSession();
        void closeSession();

        // 提交：返回 future；若 async=false，调用者可自行等待
        std::future<CaptureResult> submit(const CaptureRequest& req, ResultCallback cb);

    private:
        CameraService() = default;
        std::mutex m_;
        bool opened_{false};
        GraphExecutor exec_;
        std::shared_ptr<PipelineBus> bus_;
        std::shared_ptr<INode> source_node_;
        ISource* source_iface_{nullptr};
    };

}  // namespace mc
