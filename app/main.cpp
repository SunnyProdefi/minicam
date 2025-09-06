#include "service/camera_service.h"
#include "common/log.h"
#include <future>

using namespace mc;

int main()
{
    // 只需初始化一次：目录 logs、前缀 minicam、控制台彩色开启
    mc::Logger::instance().init("logs", "minicam", true);
    mc::Logger::instance().setLevel(mc::LogLevel::DEBUG);

    auto& svc = CameraService::instance();
    if (!svc.openSession())
    {
        LOG_ERROR() << "Failed to open camera session";
        return -1;
    }

    CaptureRequest req;
    req.request_id = 42;
    req.streams.push_back({640, 480, PixelFormat::RGBA8888, "PREVIEW"});
    req.settings.ints["frames"] = 5;  // 产生5帧
    req.async = true;

    auto fut = svc.submit(req, [](const CaptureResult& r) { LOG_INFO() << "Callback: request " << r.request_id << " done, path=" << r.artifact_path; });

    // 同步等待（可选）
    auto result = fut.get();
    LOG_INFO() << "Future: status=" << int(result.status) << ", path=" << result.artifact_path;

    svc.closeSession();
    return 0;
}
