#pragma once
#include "../core/request.h"

namespace mc
{
    // Source 请求入口接口
    class ISource
    {
    public:
        virtual ~ISource() = default;
        virtual void onRequest(const CaptureRequest& req, ResultCallback cb) = 0;
    };
}  // namespace mc
